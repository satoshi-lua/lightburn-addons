#pragma once

static void* visibleEditWindow(void* object,int depth=0) {
 if(depth>40)return nullptr;
 if(qt<bool(*)(void*,const char*)>(L"Qt6Core.dll","?inherits@QObject@@QEBA_NPEBD@Z")(object,"EditWindow")&&
    qt<bool(*)(void*)>(L"Qt6Widgets.dll","?isVisible@QWidget@@QEBA_NXZ")(object))return object;
 auto children=qt<const List*(*)(void*)>(L"Qt6Core.dll","?children@QObject@@QEBAAEBV?$QList@PEAVQObject@@@@XZ")(object);
 for(int64_t i=0;i<children->size;++i)
  if(auto editor=visibleEditWindow(children->data[i],depth+1))return editor;
 return nullptr;
}

class GeometryReference {
 static constexpr int toolLayer=30;
 void* project=nullptr;
 void* shape=nullptr;
 const List& originals;
 static void* currentProject() {return *reinterpret_cast<void**>(base+profile->projectPointer);}
 static void* selection() {
  auto gui=*reinterpret_cast<byte**>(base+profile->editorContext);
  return gui?gui+0xe8:nullptr;
 }
 bool contains(void* object) const {
  return fn<bool(*)(void*,void*)>(profile->containsShape)(project,object);
 }
 static void submit(void* node) {
  fn<void(*)(void*,void*)>(profile->submitUndo)(reinterpret_cast<void*>(base+profile->undoManager),node);
 }
 static void notify() {fn<bool(*)(int)>(profile->sendEvent)(1001);}
 class GuideVisibility {
  void* project;
  void* settings=nullptr;
  byte original=0;
  static void* currentSettings() {
   return fn<void*(*)(int,bool)>(profile->getCutSetting)(toolLayer,false);
  }
  static void setHidden(void* setting,byte hidden) {
   // Both supported builds use this CutSetting field and these notifications
   // for the native Show switch. Avoid a visibility entry in the undo stack.
   auto& state=at<byte>(setting,0x6a);
   if(state==hidden)return;
   state=hidden;
   fn<bool(*)(int)>(profile->sendEvent)(1010);
   fn<bool(*)(int,void*)>(profile->sendEventWithData)(1017,setting);
  }
 public:
  explicit GuideVisibility(void* owner):project(owner){}
  void show(void* setting) {
   original=at<byte>(setting,0x6a);
   settings=setting;
   setHidden(settings,0);
  }
  ~GuideVisibility() {
   if(settings&&currentProject()==project&&currentSettings()==settings)setHidden(settings,original);
  }
 } visibility;
public:
 GeometryReference(const Session& source,dtw::Point size):project(currentProject()),originals(source.originals),visibility(project) {
  auto selected=selection();
  if(!project||!selected)throw std::runtime_error("The editor is unavailable.");
  auto layer=fn<void*(*)(int)>(profile->getLayer)(toolLayer);
  auto settings=fn<void*(*)(int,bool)>(profile->getCutSetting)(toolLayer,false);
  if(!layer||!settings||!fn<bool(*)(void*)>(profile->isToolLayer)(settings))
   throw std::runtime_error("The T1 tool layer is unavailable.");
  visibility.show(settings);
  float center[]={float((source.minx+source.maxx)/2),float((source.miny+source.maxy)/2)};
  float dimensions[]={float(size.x),float(size.y)};
  auto node=fn<void*(*)(void*,float*,float*,void*,float)>(profile->createRect)(allocate(0x28),center,dimensions,layer,0);
  shape=at<void*>(node,0x20);
  submit(node);
  fn<void(*)(void*)>(profile->clearSelection)(selected);
  fn<bool(*)(void*,void*)>(profile->addSelection)(selected,shape);
  notify();
 }
 GeometryReference(const GeometryReference&)=delete;
 GeometryReference& operator=(const GeometryReference&)=delete;
 ~GeometryReference() {
  if(currentProject()!=project)return;
  if(contains(shape))submit(fn<void*(*)(void*,void*,bool)>(profile->deleteOneShape)(allocate(0x68),shape,false));
  if(auto selected=selection()) {
   fn<void(*)(void*)>(profile->clearSelection)(selected);
   for(int64_t i=0;i<originals.size;++i)if(contains(originals.data[i]))
    fn<bool(*)(void*,void*)>(profile->addSelection)(selected,originals.data[i]);
  }
  notify();
 }
 std::optional<dtw::Point> center() const {
  if(currentProject()!=project||!contains(shape))return std::nullopt;
  // GetBounds can reuse cached extents. Rebuild them from the rectangle's
  // current transform before capturing the center.
  fn<void(*)(void*)>(profile->computeRectBounds)(shape);
  const auto bounds=fn<const float*(*)(void*)>(profile->getBounds)(shape);
  dtw::Point result{double(bounds[0])+double(bounds[2])/2,double(bounds[1])+double(bounds[3])/2};
  if(!dtw::validPosition(result))return std::nullopt;
  return result;
 }
 static void refreshSelection() {
  // Placement may start just as the user switches to another application.
  DWORD foregroundProcess=0;
  GetWindowThreadProcessId(GetForegroundWindow(),&foregroundProcess);
  if(foregroundProcess!=GetCurrentProcessId()||!IsWindowEnabled(mainHwnd))return;
  // Restore the active top-level window before setting focus on its canvas.
  qt<void(*)(void*)>(L"Qt6Widgets.dll","?activateWindow@QWidget@@QEAAXXZ")(mainWidget);
  if(auto editor=visibleEditWindow(mainWidget)) {
   fn<void(*)(void*)>(profile->updateToolSelection)(editor);
   qt<void(*)(void*)>(L"Qt6Widgets.dll","?setFocus@QWidget@@QEAAXXZ")(editor);
  }
 }
 std::optional<dtw::Rotation> rotation(dtw::Rotation initial,bool yUp) const {
  if(currentProject()!=project||!contains(shape))return std::nullopt;
  // RectShape's transform begins at 0x88. The second basis vector
  // transforms the rectangle's local Y axis in both supported builds.
  return dtw::referenceRotation({at<float>(shape,0x90),at<float>(shape,0x94)},initial,yUp);
 }
};
