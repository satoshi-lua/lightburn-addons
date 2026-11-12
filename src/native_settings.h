#pragma once
// Requires the host Qt ABI helpers from native_bridge.cpp/native_dialog.h.
struct NativeSettings {
 void* object;
 std::string prefix;
 explicit NativeSettings(bool single):prefix(single?"TaperWarpV2/":"DualTaperWarp/") {
  NativeString path((home/L"taper-tools.ini").u8string());object=allocate(0x100);
  qt<void*(*)(void*,const void*,int,void*)>(L"Qt6Core.dll","??0QSettings@@QEAA@AEBVQString@@W4Format@0@PEAVQObject@@@Z")(object,&path,1,nullptr);
 }
 ~NativeSettings(){deleteQtObject(object);}
 double get(const char* key,double fallback) {
  NativeString name(prefix+key);alignas(16) byte def[64]{},value[64]{};
  qt<void*(*)(void*,double)>(L"Qt6Core.dll","??0QVariant@@QEAA@N@Z")(def,fallback);
  qt<void*(*)(void*,void*,const void*,const void*)>(L"Qt6Core.dll","?value@QSettings@@QEBA?AVQVariant@@AEBVQString@@AEBV2@@Z")(object,value,&name,def);
  bool ok=false;double v=qt<double(*)(void*,bool*)>(L"Qt6Core.dll","?toDouble@QVariant@@QEBANPEA_N@Z")(value,&ok);
  qt<void(*)(void*)>(L"Qt6Core.dll","??1QVariant@@QEAA@XZ")(value);qt<void(*)(void*)>(L"Qt6Core.dll","??1QVariant@@QEAA@XZ")(def);
  return ok&&std::isfinite(v)?v:fallback;
 }
 void set(const char* key,double number) {
  NativeString name(prefix+key);alignas(16) byte value[64]{};
  qt<void*(*)(void*,double)>(L"Qt6Core.dll","??0QVariant@@QEAA@N@Z")(value,number);
  qt<void(*)(void*,const void*,const void*)>(L"Qt6Core.dll","?setValue@QSettings@@QEAAXAEBVQString@@AEBVQVariant@@@Z")(object,&name,value);
  qt<void(*)(void*)>(L"Qt6Core.dll","??1QVariant@@QEAA@XZ")(value);
 }
 void geometry(void* dialog,bool save) {
  NativeString key(prefix+"windowGeometry");alignas(16) byte bytes[32]{},value[64]{};
  if(save) {
   qt<void*(*)(void*,void*)>(L"Qt6Widgets.dll","?saveGeometry@QWidget@@QEBA?AVQByteArray@@XZ")(dialog,bytes);
   qt<void*(*)(void*,const void*)>(L"Qt6Core.dll","??0QVariant@@QEAA@AEBVQByteArray@@@Z")(value,bytes);
   qt<void(*)(void*,const void*,const void*)>(L"Qt6Core.dll","?setValue@QSettings@@QEAAXAEBVQString@@AEBVQVariant@@@Z")(object,&key,value);
  } else {
   qt<void*(*)(void*,void*,const void*)>(L"Qt6Core.dll","?value@QSettings@@QEBA?AVQVariant@@AEBVQString@@@Z")(object,value,&key);
   qt<void*(*)(void*,void*)>(L"Qt6Core.dll","?toByteArray@QVariant@@QEBA?AVQByteArray@@XZ")(value,bytes);
   if(at<int64_t>(bytes,16)>0)qt<bool(*)(void*,const void*)>(L"Qt6Widgets.dll","?restoreGeometry@QWidget@@QEAA_NAEBVQByteArray@@@Z")(dialog,bytes);
  }
  qt<void(*)(void*)>(L"Qt6Core.dll","??1QVariant@@QEAA@XZ")(value);qt<void(*)(void*)>(L"Qt6Core.dll","??1QByteArray@@QEAA@XZ")(bytes);
 }
};
