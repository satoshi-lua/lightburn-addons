#pragma once
#include <functional>
#include <array>
#include "preview_svg.h"

// Qt objects are constructed/destructed by the host's Qt exports.
// Never share Qt C++ objects with a different Qt runtime or C++ ABI.
struct NativeString: String {
 explicit NativeString(const std::string& s){stringInit(*this,s.c_str());}
 ~NativeString(){stringFree(*this);}
};
static std::string utf8(const String& s) {
 const auto p=at<const wchar_t*>(const_cast<String*>(&s),8);
 auto n=at<int64_t>(const_cast<String*>(&s),16);
 if(!p||n<=0||n>32768)return {};
 int size=WideCharToMultiByte(CP_UTF8,0,p,int(n),nullptr,0,nullptr,nullptr);
 std::string text(size,'\0');WideCharToMultiByte(CP_UTF8,0,p,int(n),text.data(),size,nullptr,nullptr);return text;
}
#include "native_signal.h"
static void setText(void* label,const std::string& text) {
 NativeString s(text);qt<void(*)(void*,const void*)>(L"Qt6Widgets.dll","?setText@QLabel@@QEAAXAEBVQString@@@Z")(label,&s);
}
static void addWidget(void* layout,void* widget,int stretch=0) {
 qt<void(*)(void*,void*,int,int)>(L"Qt6Widgets.dll","?addWidget@QBoxLayout@@QEAAXPEAVQWidget@@HV?$QFlags@W4AlignmentFlag@Qt@@@@@Z")(layout,widget,stretch,0);
}
static void addLayout(void* layout,void* child,int stretch=0) {
 qt<void(*)(void*,void*,int)>(L"Qt6Widgets.dll","?addLayout@QBoxLayout@@QEAAXPEAVQLayout@@H@Z")(layout,child,stretch);
}
static void* boxLayout(bool vertical,void* parent=nullptr) {
 auto object=allocate(0x100);
 const char* name=vertical?(parent?"??0QVBoxLayout@@QEAA@PEAVQWidget@@@Z":"??0QVBoxLayout@@QEAA@XZ"):
                           (parent?"??0QHBoxLayout@@QEAA@PEAVQWidget@@@Z":"??0QHBoxLayout@@QEAA@XZ");
 if(parent)qt<void*(*)(void*,void*)>(L"Qt6Widgets.dll",name)(object,parent);
 else qt<void*(*)(void*)>(L"Qt6Widgets.dll",name)(object);
 return object;
}
static void* label(void* parent,const std::string& value) {
 NativeString s(value);auto object=allocate(0x100);
 qt<void*(*)(void*,const void*,void*,int)>(L"Qt6Widgets.dll","??0QLabel@@QEAA@AEBVQString@@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z")(object,&s,parent,0);
 return object;
}
static void* groupBox(void* parent,const std::string& value) {
 NativeString s(value);auto object=allocate(0x100);
 qt<void*(*)(void*,const void*,void*)>(L"Qt6Widgets.dll","??0QGroupBox@@QEAA@AEBVQString@@PEAVQWidget@@@Z")(object,&s,parent);
 return object;
}
static void* button(void* parent,const std::string& value,std::function<void()> clicked={}) {
 NativeString s(value);auto object=allocate(0x100);
 qt<void*(*)(void*,const void*,void*)>(L"Qt6Widgets.dll","??0QPushButton@@QEAA@AEBVQString@@PEAVQWidget@@@Z")(object,&s,parent);
 qt<void(*)(void*,bool)>(L"Qt6Widgets.dll","?setAutoDefault@QPushButton@@QEAAX_N@Z")(object,false);
 if(clicked)connectCallback(object,"?clicked@QAbstractButton@@QEAAX_N@Z","?staticMetaObject@QAbstractButton@@2UQMetaObject@@B",L"Qt6Widgets.dll",parent,std::move(clicked));
 return object;
}
static void setEnabled(void* widget,bool value) {qt<void(*)(void*,bool)>(L"Qt6Widgets.dll","?setEnabled@QWidget@@QEAAX_N@Z")(widget,value);}
static double spinValue(void* spin) {return spin?qt<double(*)(void*)>(L"Qt6Widgets.dll","?value@QDoubleSpinBox@@QEBANXZ")(spin):0;}
static void spinSet(void* spin,double value) {qt<void(*)(void*,double)>(L"Qt6Widgets.dll","?setValue@QDoubleSpinBox@@QEAAXN@Z")(spin,value);}
static void spinRange(void* spin,double low,double high) {qt<void(*)(void*,double,double)>(L"Qt6Widgets.dll","?setRange@QDoubleSpinBox@@QEAAXNN@Z")(spin,low,high);}
static int sliderValue(void* slider) {return qt<int(*)(void*)>(L"Qt6Widgets.dll","?value@QAbstractSlider@@QEBAHXZ")(slider);}
static void sliderSet(void* slider,int value) {qt<void(*)(void*,int)>(L"Qt6Widgets.dll","?setValue@QAbstractSlider@@QEAAXH@Z")(slider,value);}
static bool checked(void* check) {return qt<bool(*)(void*)>(L"Qt6Widgets.dll","?isChecked@QAbstractButton@@QEBA_NXZ")(check);}

#include "native_settings.h"

static void* taperMenu=nullptr;
struct NativeDialog;
static NativeDialog* activeDialog=nullptr;
struct NativeDialog {
 Session& source;
 NativeSettings settings;
 bool english=false,original=false,updating=false,failed=false;
 double unitScale=1,lastOffset=-1;
 int lastSlider=-1;
 void* dialog=nullptr;
 std::array<void*,6> spins{};
 void *positionSlider=nullptr,*curve=nullptr,*curveText=nullptr,*transition=nullptr,*preview=nullptr,*error=nullptr,*apply=nullptr,*zones=nullptr;
 std::string unit="mm",lastRenderKey;
 UINT_PTR timer=0;
 NativeDialog(Session& s):source(s),settings(s.single) {
  if(taperMenu) {
   String title;qt<void*(*)(void*,void*)>(L"Qt6Widgets.dll","?title@QMenu@@QEBA?AVQString@@XZ")(taperMenu,&title);
   auto name=utf8(title);stringFree(title);name.erase(std::remove(name.begin(),name.end(),'&'),name.end());english=name.find("Laser Tools")!=std::string::npos;
  }
  float scale=*reinterpret_cast<float*>(base+profile->unitScale);
  if(std::isfinite(scale)&&scale>0&&scale<1000)unitScale=scale;
  auto text=utf8(*reinterpret_cast<String*>(base+profile->unitSuffix));
  if(!text.empty()) {unit=text;while(!unit.empty()&&unit.front()==' ')unit.erase(unit.begin());}
  else if(std::abs(unitScale-1)>.0001)unit="in";
  try {build();}catch(...) {if(dialog){deleteQtObject(dialog);dialog=nullptr;}throw;}
 }
 ~NativeDialog(){if(timer)KillTimer(nullptr,timer);activeDialog=nullptr;if(dialog)deleteQtObject(dialog);}
 std::string tr(const char* es,const char* en)const{return english?en:es;}
 void row(void* form,const std::string& name,void* widget) {
  NativeString text(name);qt<void(*)(void*,const void*,void*)>(L"Qt6Widgets.dll","?addRow@QFormLayout@@QEAAXAEBVQString@@PEAVQWidget@@@Z")(form,&text,widget);
 }
 void* spin(void* parent,double value,bool zeroAllowed=false) {
  auto object=allocate(0x100);qt<void*(*)(void*,void*)>(L"Qt6Widgets.dll","??0QDoubleSpinBox@@QEAA@PEAVQWidget@@@Z")(object,parent);
  qt<void(*)(void*,int)>(L"Qt6Widgets.dll","?setDecimals@QDoubleSpinBox@@QEAAXH@Z")(object,std::abs(unitScale-1)<.0001?2:4);
  spinRange(object,zeroAllowed?0:.01*unitScale,10000*unitScale);
  NativeString suffix(" "+unit);qt<void(*)(void*,const void*)>(L"Qt6Widgets.dll","?setSuffix@QDoubleSpinBox@@QEAAXAEBVQString@@@Z")(object,&suffix);
  spinSet(object,value*unitScale);return object;
 }
 void* slider(void* parent,int maximum,int value) {
  auto object=allocate(0x100);qt<void*(*)(void*,int,void*)>(L"Qt6Widgets.dll","??0QSlider@@QEAA@W4Orientation@Qt@@PEAVQWidget@@@Z")(object,1,parent);
  qt<void(*)(void*,int,int)>(L"Qt6Widgets.dll","?setRange@QAbstractSlider@@QEAAXHH@Z")(object,0,maximum);sliderSet(object,value);return object;
 }
 void build() {
  dialog=allocate(0x100);qt<void*(*)(void*,void*,int)>(L"Qt6Widgets.dll","??0QDialog@@QEAA@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z")(dialog,mainWidget,0);
  NativeString title(source.single?"Taper Warp v2":"Dual Taper Warp");
  qt<void(*)(void*,const void*)>(L"Qt6Widgets.dll","?setWindowTitle@QWidget@@QEAAXAEBVQString@@@Z")(dialog,&title);
  qt<void(*)(void*,int,int)>(L"Qt6Widgets.dll","?resize@QWidget@@QEAAXHH@Z")(dialog,1100,730);
  auto root=boxLayout(true,dialog),body=boxLayout(false),left=boxLayout(true),right=boxLayout(true);
  addLayout(root,body,1);addLayout(body,left);addLayout(body,right,1);
  auto group=groupBox(dialog,tr("Geometría del recipiente","Vessel geometry"));
  auto form=allocate(0x100);qt<void*(*)(void*,void*)>(L"Qt6Widgets.dll","??0QFormLayout@@QEAA@PEAVQWidget@@@Z")(form,group);
  const double defaults[]={source.single?87.:80.,source.single?0.:60.,source.single?73.:70.,source.single?0.:20.,source.single?143.:108.};
  const char* keys[]={"topDiameter","upperStraight","bottomDiameter","lowerStraight","referenceHeight"};
  const std::string names[]={tr("Diámetro superior","Top diameter"),tr("Recta superior","Upper straight"),tr("Diámetro inferior","Bottom diameter"),tr("Recta inferior","Lower straight"),source.single?tr("Altura de referencia","Reference height"):tr("Altura total","Total height")};
  for(int i=0;i<5;i++)if(!source.single||i==0||i==2||i==4) {
   spins[i]=spin(group,settings.get(keys[i],defaults[i]),i==1||i==3);row(form,names[i],spins[i]);
  }
  if(!source.single) {
   curve=slider(group,100,int(std::clamp(settings.get("curvePercent",49),0.,100.)));
   curveText=label(group,"");auto cr=boxLayout(false);addWidget(cr,curve,1);addWidget(cr,curveText);
   NativeString name(tr("Intensidad de curva","Curve strength"));
   qt<void(*)(void*,const void*,void*)>(L"Qt6Widgets.dll","?addRow@QFormLayout@@QEAAXAEBVQString@@PEAVQLayout@@@Z")(form,&name,cr);
   transition=label(group,"");row(form,tr("Transición","Transition"),transition);
  }
  addWidget(left,group);
  auto position=groupBox(dialog,tr("Posición del diseño","Design position")),pl=boxLayout(true,position);
  addWidget(pl,label(position,tr("Desde el borde superior","Distance from top")));
  const double H=spinValue(spins[4])/unitScale;
  const double maxOffset=std::max(0.,H-(source.maxy-source.miny));
  spins[5]=spin(position,std::clamp(settings.get("offset",maxOffset/2),0.,maxOffset),true);spinRange(spins[5],0,maxOffset*unitScale);
  addWidget(pl,spins[5]);positionSlider=slider(position,10000,maxOffset?int(spinValue(spins[5])/unitScale/maxOffset*10000):0);addWidget(pl,positionSlider);
  lastOffset=spinValue(spins[5]);lastSlider=sliderValue(positionSlider);
  addWidget(pl,button(position,tr("Centrar verticalmente","Center vertically"),[this]{spinSet(spins[5],std::max(0.,spinValue(spins[4])-(source.maxy-source.miny)*unitScale)/2);update();}));
  addWidget(pl,label(position,tr("La altura del diseño se conserva.","Design height is preserved.")));addWidget(left,position);
  std::ostringstream dims;dims<<std::fixed<<std::setprecision(2)<<(source.maxx-source.minx)*unitScale<<" × "<<(source.maxy-source.miny)*unitScale<<" "<<unit;
  addWidget(left,label(dialog,tr("Selección: ","Selection: ")+dims.str()));
  auto note=label(dialog,tr("Los vectores se convierten en trazados subdivididos.","Vectors are converted to subdivided paths."));
  qt<void(*)(void*,bool)>(L"Qt6Widgets.dll","?setWordWrap@QLabel@@QEAAX_N@Z")(note,true);addWidget(left,note);
  error=label(dialog,"");qt<void(*)(void*,bool)>(L"Qt6Widgets.dll","?setWordWrap@QLabel@@QEAAX_N@Z")(error,true);addWidget(left,error);
  qt<void(*)(void*,int)>(L"Qt6Widgets.dll","?addStretch@QBoxLayout@@QEAAXH@Z")(left,1);
  addWidget(right,label(dialog,tr("Vista previa","Preview")));
  preview=label(dialog,"");qt<void(*)(void*,int,int)>(L"Qt6Widgets.dll","?setMinimumSize@QWidget@@QEAAXHH@Z")(preview,610,480);
  qt<void(*)(void*,int,int)>(L"Qt6Widgets.dll","?setSizePolicy@QWidget@@QEAAXW4Policy@QSizePolicy@@0@Z")(preview,13,13);
  qt<void(*)(void*,int)>(L"Qt6Widgets.dll","?setAlignment@QLabel@@QEAAXV?$QFlags@W4AlignmentFlag@Qt@@@@@Z")(preview,0x84);addWidget(right,preview,1);
  auto options=boxLayout(false);
  auto hold=button(dialog,tr("Mantener para ver original","Hold to view original"));addWidget(options,hold);
  connectCallback(hold,"?pressed@QAbstractButton@@QEAAXXZ","?staticMetaObject@QAbstractButton@@2UQMetaObject@@B",L"Qt6Widgets.dll",dialog,[this]{original=true;update();});
  connectCallback(hold,"?released@QAbstractButton@@QEAAXXZ","?staticMetaObject@QAbstractButton@@2UQMetaObject@@B",L"Qt6Widgets.dll",dialog,[this]{original=false;update();});
  NativeString ztext(source.single?tr("Mostrar referencia","Show reference"):tr("Mostrar zonas","Show zones"));
  zones=allocate(0x100);qt<void*(*)(void*,const void*,void*)>(L"Qt6Widgets.dll","??0QCheckBox@@QEAA@AEBVQString@@PEAVQWidget@@@Z")(zones,&ztext,dialog);
  qt<void(*)(void*,bool)>(L"Qt6Widgets.dll","?setChecked@QAbstractButton@@QEAAX_N@Z")(zones,settings.get("showZones",1)!=0);
  addWidget(options,zones);addLayout(right,options);
  auto buttons=allocate(0x100);qt<void*(*)(void*,int,void*)>(L"Qt6Widgets.dll","??0QDialogButtonBox@@QEAA@V?$QFlags@W4StandardButton@QDialogButtonBox@@@@PEAVQWidget@@@Z")(buttons,0x400|0x400000,dialog);
  apply=qt<void*(*)(void*,int)>(L"Qt6Widgets.dll","?button@QDialogButtonBox@@QEBAPEAVQPushButton@@W4StandardButton@1@@Z")(buttons,0x400);
  NativeString atext(tr("Aplicar","Apply"));qt<void(*)(void*,const void*)>(L"Qt6Widgets.dll","?setText@QAbstractButton@@QEAAXAEBVQString@@@Z")(apply,&atext);
  connectCallback(buttons,"?accepted@QDialogButtonBox@@QEAAXXZ","?staticMetaObject@QDialogButtonBox@@2UQMetaObject@@B",L"Qt6Widgets.dll",dialog,[this]{applyChanges();});
  connectCallback(buttons,"?rejected@QDialogButtonBox@@QEAAXXZ","?staticMetaObject@QDialogButtonBox@@2UQMetaObject@@B",L"Qt6Widgets.dll",dialog,[this]{qt<void(*)(void*)>(L"Qt6Widgets.dll","?reject@QDialog@@UEAAXXZ")(dialog);});
  addWidget(root,buttons);settings.geometry(dialog,false);
 }
 Params parameters()const {
  return {spinValue(spins[0])/unitScale,spinValue(spins[1])/unitScale,
          spinValue(spins[2])/unitScale,spinValue(spins[3])/unitScale,
          spinValue(spins[4])/unitScale,curve?sliderValue(curve)/100.:0.,spinValue(spins[5])/unitScale};
 }
 dtw::PreviewTheme theme()const {
  auto palette=qt<const void*(*)(void*)>(L"Qt6Widgets.dll","?palette@QWidget@@QEBAAEBVQPalette@@XZ")(preview);
  auto color=[&](int role){auto c=qt<const void*(*)(const void*,int,int)>(L"Qt6Gui.dll","?color@QPalette@@QEBAAEBVQColor@@W4ColorGroup@1@W4ColorRole@1@@Z")(palette,0,role);
   unsigned rgba=qt<unsigned(*)(const void*)>(L"Qt6Gui.dll","?rgba@QColor@@QEBAIXZ")(c);
   std::ostringstream s;s<<"#"<<std::hex<<std::setw(6)<<std::setfill('0')<<(rgba&0xffffff);return s.str();};
  return {color(9),color(0),color(12)};
 }
 void update() {
  if(updating||failed)return;updating=true;
  try {
   double maxOffset=std::max(0.,spinValue(spins[4])-(source.maxy-source.miny)*unitScale);
   spinRange(spins[5],0,maxOffset);
   int sv=sliderValue(positionSlider);double ov=spinValue(spins[5]);
   if(sv!=lastSlider&&ov==lastOffset) {spinSet(spins[5],maxOffset*sv/10000.);ov=spinValue(spins[5]);}
   else {sv=maxOffset?int(std::lround(ov/maxOffset*10000)):0;sliderSet(positionSlider,sv);}
   lastOffset=ov;lastSlider=sv;auto p=parameters();
   if(curveText)setText(curveText,std::to_string(sliderValue(curve))+" %");
   if(transition) {std::ostringstream value;value<<std::fixed<<std::setprecision(2)<<(p.height-p.upper-p.lower)*unitScale<<" "<<unit;setText(transition,value.str());}
   try {validate(p,source);setText(error,"");setEnabled(apply,true);}
   catch(const std::exception&) {setText(error,tr("Revisa las medidas: la transición debe ser positiva y el diseño debe caber en la referencia.","Check dimensions: the transition must be positive and the design must fit the reference."));setEnabled(apply,false);updating=false;return;}
   auto r=qt<const int*(*)(void*)>(L"Qt6Widgets.dll","?geometry@QWidget@@QEBAAEBVQRect@@XZ")(preview);
   int width=std::clamp(r[2]-r[0]+1,610,2400),height=std::clamp(r[3]-r[1]+1,480,1800);
   const auto colors=theme();bool showZones=checked(zones);
   std::ostringstream key;key.imbue(std::locale::classic());key<<std::setprecision(17)<<p.top<<','<<p.upper<<','<<p.bottom<<','<<p.lower<<','<<p.height<<','<<p.curve<<','<<p.offset<<','<<width<<','<<height<<','<<original<<','<<showZones<<','<<colors.background<<colors.text<<colors.accent;
   if(key.str()!=lastRenderKey) {
    auto svg=dtw::previewSvg(p,source.single,source.maxx-source.minx,source.previewPaths,width,height,original,showZones,english,unitScale,unit,colors);
    alignas(16) byte pixmap[128]{};
    qt<void*(*)(void*)>(L"Qt6Gui.dll","??0QPixmap@@QEAA@XZ")(pixmap);
    bool loaded=qt<bool(*)(void*,const byte*,unsigned,const char*,int)>(L"Qt6Gui.dll","?loadFromData@QPixmap@@QEAA_NPEBEIPEBDV?$QFlags@W4ImageConversionFlag@Qt@@@@@Z")(pixmap,reinterpret_cast<const byte*>(svg.data()),unsigned(svg.size()),"SVG",0);
    if(loaded)qt<void(*)(void*,const void*)>(L"Qt6Widgets.dll","?setPixmap@QLabel@@QEAAXAEBVQPixmap@@@Z")(preview,pixmap);
    qt<void(*)(void*)>(L"Qt6Gui.dll","??1QPixmap@@UEAA@XZ")(pixmap);
    if(!loaded)throw std::runtime_error(tr("El renderizador SVG de Qt no pudo dibujar la vista previa.","Qt could not render the SVG preview."));
    lastRenderKey=key.str();
   }
  }catch(const std::exception& e){log(e.what());setText(error,e.what());setEnabled(apply,false);failed=true;}
  updating=false;
 }
 void applyChanges() {
  try {auto p=parameters();validate(p,source);commit(source,p);qt<void(*)(void*)>(L"Qt6Widgets.dll","?accept@QDialog@@UEAAXXZ")(dialog);}
  catch(const std::exception& e) {
   if(undoGroup){fn<void(*)(void*)>(profile->rollbackUndo)(undoGroup);deleteShape(undoGroup);undoGroup=nullptr;}
   log(e.what());setText(error,e.what());setEnabled(apply,false);failed=true;
  }
 }
 void save() {
  settings.geometry(dialog,true);
  try {
   auto p=parameters();validate(p,source);
   const char* keys[]={"topDiameter","upperStraight","bottomDiameter","lowerStraight","referenceHeight","offset"};
   double values[]={p.top,p.upper,p.bottom,p.lower,p.height,p.offset};
   for(int i=0;i<6;i++)settings.set(keys[i],values[i]);
   settings.set("curvePercent",p.curve*100);settings.set("showZones",checked(zones)?1:0);
  }catch(const std::exception&){/* Keep previous valid dimensions. */}
 }
 static void CALLBACK tick(HWND,UINT,UINT_PTR,DWORD){if(activeDialog)activeDialog->update();}
 int exec() {
  activeDialog=this;update();timer=SetTimer(nullptr,0,125,tick);
  if(!timer)throw std::runtime_error(tr("No se pudo actualizar la vista previa.","Could not update the preview."));
  int result=qt<int(*)(void*)>(L"Qt6Widgets.dll","?exec@QDialog@@UEAAHXZ")(dialog);
  KillTimer(nullptr,timer);timer=0;save();return result;
 }
};
