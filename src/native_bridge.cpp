// Native adapter for exact LightBurn Windows x64 builds.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "request_io.h"
#include "compensation.h"
#include "preview_svg.h"
#include "presets.h"
#include <optional>
#include "verified_image.h"

using byte=unsigned char;
namespace fs=std::filesystem;
struct List { void* header{}; void** data{}; int64_t size{}; };
static_assert(sizeof(List)==24);
struct alignas(void*) String { byte storage[24]{}; };
struct Element { double x,y; int type; int pad; };
using Params=dtw::Parameters;
static HMODULE module;
static uintptr_t base;
static const lightburn::Profile* profile;
static HWND mainHwnd;
static void* mainWidget;
static HHOOK hook;
static bool initialized=false, opening=false;
static fs::path home;
static std::vector<void*> menus;

template<class T> T fn(uintptr_t rva) {return reinterpret_cast<T>(base+rva);}
template<class T> T qt(const wchar_t* dll,const char* name) {
 auto p=GetProcAddress(GetModuleHandleW(dll),name);
 if(!p) throw std::runtime_error(std::string("Qt export unavailable: ")+name);
 return reinterpret_cast<T>(p);
}
template<class T> T& at(void* p,size_t o) {return *reinterpret_cast<T*>(static_cast<byte*>(p)+o);}
static void log(const std::string& message) {
 std::ofstream f(home/L"bridge.log",std::ios::app); f<<message<<"\n";
}
static void* allocate(size_t n) {return fn<void*(*)(size_t)>(profile->allocate)(n);}
static void releaseList(List& l) {fn<void(*)(void*)>(profile->releaseList)(&l); l={};}
static List retain(const List& l) {
 List r=l; if(r.header) InterlockedIncrement(static_cast<volatile LONG*>(r.header)); return r;
}
static int type(void* shape) {return reinterpret_cast<int(*)(void*)>((*reinterpret_cast<void***>(shape))[1])(shape);}
static void deleteShape(void* shape) {if(shape) reinterpret_cast<void(*)(void*,unsigned)>((*reinterpret_cast<void***>(shape))[0])(shape,1);}
// QObject declares its three meta-object virtuals before its virtual destructor.
// Its vtable layout is different from LightBurn's Shape/UndoNode hierarchy.
static void deleteQtObject(void* object) {if(object) reinterpret_cast<void(*)(void*,unsigned)>((*reinterpret_cast<void***>(object))[3])(object,1);}
static void stringInit(String& s,const char* text) {
 qt<void(*)(void*,const char*)>(L"Qt6Core.dll","??0QString@@QEAA@PEBD@Z")(&s,text);
}
static void stringFree(String& s) {qt<void(*)(void*)>(L"Qt6Core.dll","??1QString@@QEAA@XZ")(&s);}
static std::wstring objectName(void* object) {
 String s;
 qt<void*(*)(void*,void*)>(L"Qt6Core.dll","?objectName@QObject@@QEBA?AVQString@@XZ")(object,&s);
 auto p=at<wchar_t*>(&s,8); auto n=at<int64_t>(&s,16);
 std::wstring result=(p&&n>0&&n<4096)?std::wstring(p,static_cast<size_t>(n)):L"";
 stringFree(s); return result;
}
static void* findAction(void* object,int depth=0) {
 if(depth>40) return nullptr;
 if(objectName(object)==L"actionTaper_Warp") return object;
 if(qt<bool(*)(void*,const char*)>(L"Qt6Core.dll","?inherits@QObject@@QEBA_NPEBD@Z")(object,"QMenu")) menus.push_back(object);
 auto children=qt<const List*(*)(void*)>(L"Qt6Core.dll","?children@QObject@@QEBAAEBV?$QList@PEAVQObject@@@@XZ")(object);
 if(children->size<0||children->size>100000) throw std::runtime_error("Invalid QObject child list");
 void* found=nullptr;
 for(int64_t i=0;i<children->size;i++) {auto p=findAction(children->data[i],depth+1);if(p) found=p;}
 return found;
}
static bool supported(void* shape,int depth=0) {
 if(depth>30) return false;
 int t=type(shape);
 if(t==7||t==11||t==12) return false;
 if(t==10) {
  for(auto child=at<void*>(shape,0xe8);child;child=at<void*>(child,8)) if(!supported(child,depth+1)) return false;
 }
 return true;
}
struct Session {
 List originals,paths;
 bool committed=false,flip=false,single=false;
 double minx=0,miny=0,maxx=0,maxy=0;
 std::vector<dtw::PreviewPath> previewPaths;
 ~Session() {
  if(!committed) for(int64_t i=0;i<paths.size;i++) deleteShape(paths.data[i]);
  releaseList(paths);releaseList(originals);
 }
};
static Session* session=nullptr;

static void validate(const Params& p,const Session& s) {
 if(!dtw::validGeometry(p)||!std::isfinite(p.offset)||p.offset<0||p.offset+s.maxy-s.miny>p.height+1e-5)
  throw std::runtime_error("Dimensions are out of range.");
}
static void* undoGroup=nullptr;
static void addNode(void* node) {fn<void(*)(void*,void*,void*)>(profile->addUndoNode)(static_cast<byte*>(undoGroup)+0x30,nullptr,node);}
static void commit(Session& s,const Params& p,std::optional<dtw::Placement> position) {
 validate(p,s);
 auto gui=*reinterpret_cast<byte**>(base+profile->editorContext);
 auto& current=*reinterpret_cast<List*>(gui+0xe8);
 if(current.size!=s.originals.size) throw std::runtime_error("The selection changed. Reopen the tool.");
 for(int64_t i=0;i<current.size;i++) if(current.data[i]!=s.originals.data[i]) throw std::runtime_error("The selection changed.");
 double cx=(s.minx+s.maxx)/2;
 double minX=std::numeric_limits<double>::infinity(),maxX=-minX;
 for(int64_t i=0;i<s.paths.size;i++) {
  const auto& vertices=at<List>(s.paths.data[i],0xf8);
  if(vertices.size<0||vertices.size>2000000)throw std::runtime_error("Invalid vertex list.");
  const auto* data=reinterpret_cast<const byte*>(vertices.data);
  for(int64_t j=0;j<vertices.size;j++) {
   const auto* xy=reinterpret_cast<const float*>(data+28*j);
   const double y=s.flip?s.maxy-xy[1]:xy[1]-s.miny;
   const double x=dtw::warpedX(xy[0],cx,y,p,s.single);
   if(!std::isfinite(x)||std::abs(x)>1e7)throw std::runtime_error("Geometry is out of range.");
   minX=std::min(minX,x);maxX=std::max(maxX,x);
  }
 }
 const dtw::Point warpedCenter=position&&position->geometryReference
  ?dtw::referenceAnchor(cx,s.flip?s.maxy:s.miny,p,s.flip)
  :dtw::Point{(minX+maxX)/2,(s.miny+s.maxy)/2};
 if(position) {
  // Opposite corners still bound both axes after a quarter turn.
  if(!dtw::validPosition(dtw::placePoint({minX,s.miny},warpedCenter,*position,s.flip))||
     !dtw::validPosition(dtw::placePoint({maxX,s.maxy},warpedCenter,*position,s.flip)))
   throw std::runtime_error("The placed design is out of range.");
 }
 for(int64_t i=0;i<s.paths.size;i++) {
  auto path=s.paths.data[i];
  auto& vertices=at<List>(path,0xf8);
  byte* verts=reinterpret_cast<byte*>(vertices.data);
  for(int64_t j=0;j<vertices.size;j++) {
   float* xy=reinterpret_cast<float*>(verts+28*j);
   double y=s.flip?s.maxy-xy[1]:xy[1]-s.miny;
   dtw::Point result{dtw::warpedX(xy[0],cx,y,p,s.single),xy[1]};
   if(position)result=dtw::placePoint(result,warpedCenter,*position,s.flip);
   xy[0]=static_cast<float>(result.x);
   xy[1]=static_cast<float>(result.y);
  }
  fn<void(*)(void*)>(profile->computeBounds)(path);
 }
 undoGroup=fn<void*(*)(void*)>(profile->makeUndoGroup)(allocate(0x50));
 String name;stringInit(name,s.single?"Taper Warp v2":"Dual Taper Warp");
 qt<void*(*)(void*,const void*)>(L"Qt6Core.dll","??4QString@@QEAAAEAV0@AEBV0@@Z")(static_cast<byte*>(undoGroup)+0x18,&name);
 stringFree(name);
 addNode(fn<void*(*)(void*,const List*,void*,bool)>(profile->makeUndoAddShapes)(allocate(0x40),&s.paths,nullptr,false));
 addNode(fn<void*(*)(void*,const List*)>(profile->makeUndoSelection)(allocate(0x48),&s.paths));
 addNode(fn<void*(*)(void*,const List*,bool,bool)>(profile->makeUndoDeleteShapes)(allocate(0x68),&s.originals,false,false));
 fn<void(*)(void*,void*)>(profile->submitUndo)(reinterpret_cast<void*>(base+profile->undoManager),undoGroup);
 undoGroup=nullptr;s.committed=true;
 fn<bool(*)(int)>(profile->sendEvent)(1001); // Same document-change notification as Taper Warp.
}
static void finishSession() {
 delete session;session=nullptr;opening=false;
}
static void prepareSession(Session& s) {
 s.minx=s.miny=std::numeric_limits<double>::infinity();
 s.maxx=s.maxy=-std::numeric_limits<double>::infinity();
 int64_t count=0;
 for(int64_t i=0;i<s.paths.size;i++) {
  auto path=s.paths.data[i];
  fn<void(*)(void*,float)>(profile->limitPrimLength)(path,.25f);
  auto& vertices=at<List>(path,0xf8);
  count+=vertices.size;
  if(count>500000||vertices.size<0) throw std::runtime_error("The selection exceeds 500,000 subdivided vertices.");
  auto v=reinterpret_cast<byte*>(vertices.data);
  for(int64_t j=0;j<vertices.size;j++) {
   auto xy=reinterpret_cast<float*>(v+j*28);
   if(!std::isfinite(xy[0])||!std::isfinite(xy[1])) throw std::runtime_error("Invalid coordinates.");
   s.minx=std::min(s.minx,double(xy[0]));s.maxx=std::max(s.maxx,double(xy[0]));
   s.miny=std::min(s.miny,double(xy[1]));s.maxy=std::max(s.maxy,double(xy[1]));
  }
 }
 if(!count||s.maxx-s.minx<.0001||s.maxy-s.miny<.0001) throw std::runtime_error("Select artwork with a positive width and height.");
 auto coord=*reinterpret_cast<byte**>(base+profile->coordinateMode);
 if(!coord) throw std::runtime_error("Could not read the editor orientation.");
 // This flag describes the host coordinate origin, not a request to invert Y.
 // Use its complement to map the artwork into the preview's top-down axis.
 // The same Session::flip is used by commit() to place compensation zones.
 s.flip=!coord[3];
 for(int64_t i=0;i<s.paths.size;i++) {
  alignas(16) byte painterPath[64]{};
  fn<void*(*)(void*,void*)>(profile->buildPainterPath)(s.paths.data[i],painterPath);
  int n=qt<int(*)(void*)>(L"Qt6Gui.dll","?elementCount@QPainterPath@@QEBAHXZ")(painterPath);
  dtw::PreviewPath points;
  points.reserve(n);
  for(int j=0;j<n;j++) {
   Element e{};
   qt<void*(*)(void*,Element*,int)>(L"Qt6Gui.dll","?elementAt@QPainterPath@@QEBA?AVElement@1@H@Z")(painterPath,&e,j);
   if(e.type>1) {
    qt<void(*)(void*)>(L"Qt6Gui.dll","??1QPainterPath@@QEAA@XZ")(painterPath);
    throw std::runtime_error("Subdivision did not produce line segments.");
   }
   points.push_back({e.type,e.x-(s.minx+s.maxx)/2,s.flip?s.maxy-e.y:e.y-s.miny});
  }
  s.previewPaths.push_back(std::move(points));
  qt<void(*)(void*)>(L"Qt6Gui.dll","??1QPainterPath@@QEAA@XZ")(painterPath);
 }
}
#include "native_dialog.h"
static void openDialog(bool single) {
 if(opening)return;opening=true;
 try {
  auto gui=*reinterpret_cast<byte**>(base+profile->editorContext);
  if(!gui) throw std::runtime_error("The editor is unavailable.");
  auto& selection=*reinterpret_cast<List*>(gui+0xe8);
  if(selection.size<=0||selection.size>10000) throw std::runtime_error("Select the vector artwork you want to warp.");
  for(int64_t i=0;i<selection.size;i++) if(!supported(selection.data[i])) throw std::runtime_error("Select vectors only. Remove images and 3D models from the selection.");
  session=new Session;session->single=single;session->originals=retain(selection);
  for(int64_t i=0;i<selection.size;i++) fn<void(*)(void*,List*,float)>(profile->flatten)(selection.data[i],&session->paths,.01f);
  if(!session->paths.size) throw std::runtime_error("Could not convert the vector artwork.");
  prepareSession(*session);
  {NativeDialog dialog(*session);dialog.exec();}
  log(session->committed?"Applied native taper dialog":"Cancelled native taper dialog");
  finishSession();
 } catch(const std::exception& e) {
  if(session)finishSession();else opening=false;
  MessageBoxA(mainHwnd,e.what(),"Dual Taper Warp",MB_OK|MB_ICONINFORMATION);log(e.what());
 }
}
static void* addMenuAction(void* target,void* before,const char* name,bool single) {
 String title;stringInit(title,name);
 void* action=allocate(0x100);
 qt<void*(*)(void*,const void*,void*)>(L"Qt6Gui.dll","??0QAction@@QEAA@AEBVQString@@PEAVQObject@@@Z")(action,&title,mainWidget);
 stringFree(title);
 connectCallback(action,"?triggered@QAction@@QEAAX_N@Z","?staticMetaObject@QAction@@2UQMetaObject@@B",L"Qt6Gui.dll",mainWidget,[single]{openDialog(single);});
 qt<void(*)(void*,void*,void*)>(L"Qt6Widgets.dll","?insertAction@QWidget@@QEAAXPEAVQAction@@0@Z")(target,before,action);
 return action;
}
static void *singleAction=nullptr,*dualAction=nullptr;
static void updateMenuState() {
 bool enabled=false;
 if(!opening) {
  auto gui=*reinterpret_cast<byte**>(base+profile->editorContext);
  if(gui) {
   const auto& selected=*reinterpret_cast<const List*>(gui+0xe8);
   enabled=selected.size>0&&selected.size<=10000;
   if(enabled)for(int64_t i=0;i<selected.size;i++)if(!supported(selected.data[i])){enabled=false;break;}
  }
 }
 auto set=qt<void(*)(void*,bool)>(L"Qt6Gui.dll","?setEnabled@QAction@@QEAAX_N@Z");set(singleAction,enabled);set(dualAction,enabled);
}
static void installMenu() {
 auto taper=findAction(mainWidget);
 if(!taper)throw std::runtime_error("Could not find actionTaper_Warp.");
 void* target=nullptr;void* before=nullptr;
 for(auto menu:menus) {
  List actions;
  qt<void*(*)(void*,List*)>(L"Qt6Widgets.dll","?actions@QWidget@@QEBA?AV?$QList@PEAVQAction@@@@XZ")(menu,&actions);
  for(int64_t i=0;i<actions.size;i++)if(actions.data[i]==taper) {target=menu;before=i+1<actions.size?actions.data[i+1]:nullptr;}
  releaseList(actions);
 }
 if(!target)throw std::runtime_error("Could not find the Taper Warp menu.");
 taperMenu=target;
 singleAction=addMenuAction(target,before,"Taper Warp v2",true);
 dualAction=addMenuAction(target,before,"Dual Taper Warp",false);
 connectCallback(target,"?aboutToShow@QMenu@@QEAAXXZ","?staticMetaObject@QMenu@@2UQMetaObject@@B",L"Qt6Widgets.dll",mainWidget,[]{updateMenuState();});
 updateMenuState();
 log("Installed Taper Warp v2 and Dual Taper Warp below Taper Warp");
 std::ofstream(home/L"ready.txt")<<GetCurrentProcessId()<<" "<<lightburn::packageVersion;
}
static LRESULT CALLBACK getMessage(int code,WPARAM w,LPARAM l) {
 if(code>=0&&!initialized) {
  initialized=true;
  try {
   // QWidget lookup and traversal belong to the editor's GUI thread.
   mainWidget=qt<void*(*)(uintptr_t)>(L"Qt6Widgets.dll","?find@QWidget@@SAPEAV1@_K@Z")(reinterpret_cast<uintptr_t>(mainHwnd));
   if(!mainWidget)throw std::runtime_error("No Qt main widget");
   installMenu();
  }catch(const std::exception& e) {
   log(e.what());
   std::ofstream(home/(L"startup-"+std::to_wstring(GetCurrentProcessId())+lightburn::startupErrorSuffix))<<e.what();
   MessageBoxA(mainHwnd,e.what(),"Dual Taper Warp",MB_OK|MB_ICONERROR);
  }
  if(hook) {UnhookWindowsHookEx(hook);hook=nullptr;}
 }
 return CallNextHookEx(nullptr,code,w,l);
}
static BOOL CALLBACK findWindow(HWND w,LPARAM param) {
 DWORD pid=0;GetWindowThreadProcessId(w,&pid);
 wchar_t title[1024]{};GetWindowTextW(w,title,1024);
 if(pid==GetCurrentProcessId()&&IsWindowVisible(w)&&!GetWindow(w,GW_OWNER)
    &&std::wstring(title).find(L" - LightBurn")!=std::wstring::npos) {mainHwnd=w;return FALSE;}
 return TRUE;
}
static DWORD WINAPI initialize(void*) {
 wchar_t path[32768]{};
 const auto length=GetModuleFileNameW(module,path,32768);
 if(!length||length==32768)return 1;
 home=fs::path(path).parent_path();
 try {
  const auto hostLength=GetModuleFileNameW(nullptr,path,32768);
  if(!hostLength||hostLength==32768)throw std::runtime_error("Could not locate the host executable.");
  profile=lightburn::findProfile(lightburn::ImageFile(path).sha256());
  if(!profile)throw std::runtime_error("Unsupported LightBurn executable SHA-256.");
  base=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
  if(!lightburn::matchesMappedImage(GetCurrentProcess(),base,*profile))
   throw std::runtime_error("The mapped LightBurn image does not match its build profile.");
 }catch(const std::exception& e) {
  log(e.what());
  std::ofstream(home/(L"startup-"+std::to_wstring(GetCurrentProcessId())+lightburn::startupErrorSuffix))<<e.what();
  return 1;
 }
 log(std::string("Bridge ")+lightburn::packageVersion+" on LightBurn "+profile->version+": waiting for editor window");
 for(int attempt=0;attempt<240&&!mainHwnd;attempt++) {
  EnumWindows(findWindow,0);
  if(!mainHwnd) Sleep(250);
 }
 if(!mainHwnd){
  log("No main editor window after 60 seconds");
  std::ofstream(home/(L"startup-"+std::to_wstring(GetCurrentProcessId())+lightburn::startupErrorSuffix))<<"The editor window did not appear within 60 seconds. Close any startup dialogs and restart LightBurn.";
  return 1;
 }
 try {
  DWORD tid=GetWindowThreadProcessId(mainHwnd,nullptr);
  hook=SetWindowsHookExW(WH_GETMESSAGE,getMessage,module,tid);
  if(!hook)throw std::runtime_error("GUI hook unavailable");
  PostThreadMessageW(tid,WM_NULL,0,0);
 }catch(const std::exception& e) {
  log(e.what());
  std::ofstream(home/(L"startup-"+std::to_wstring(GetCurrentProcessId())+lightburn::startupErrorSuffix))<<e.what();
  return 1;
 }
 return 0;
}
BOOL APIENTRY DllMain(HMODULE h,DWORD reason,LPVOID) {
 if(reason==DLL_PROCESS_ATTACH) {module=h;DisableThreadLibraryCalls(h);HANDLE t=CreateThread(nullptr,0,initialize,nullptr,0,nullptr);if(t)CloseHandle(t);}
 return TRUE;
}
