#pragma once
#include <functional>
struct CallbackSlot {
 LONG ref=1;
 void(*impl)(int,CallbackSlot*,void*,void**,bool*)=nullptr;
 std::function<void()> callback;
};
static void callbackImpl(int op,CallbackSlot* self,void*,void**,bool* result) {
 if(op==0)delete self;
 else if(op==1) {try{self->callback();}catch(const std::exception& e){log(e.what());}}
 else if(op==2&&result)*result=false;
}
static void connectCallback(void* sender,const char* signalName,const char* metaName,
                            const wchar_t* library,void* context,std::function<void()> callback) {
 // Qt's MSVC build compares the full member-function pointer representation,
 // including the adjustment fields. Do not pass an isolated 8-byte address.
 struct MemberSignal {void* address;int thisAdjustment;int virtualAdjustment;};
 static_assert(sizeof(MemberSignal)==16);
 MemberSignal signal{qt<void*>(library,signalName),0,0};
 auto meta=qt<void*>(library,metaName);
 auto slot=new CallbackSlot;slot->impl=callbackImpl;slot->callback=std::move(callback);
 void* connection=nullptr;
 qt<void*(*)(void*,void*,void*,void*,void**,CallbackSlot*,int,const int*,void*)>(L"Qt6Core.dll","?connectImpl@QObject@@CA?AVConnection@QMetaObject@@PEBV1@PEAPEAX01PEAVQSlotObjectBase@QtPrivate@@W4ConnectionType@Qt@@PEBHPEBU3@@Z")(&connection,sender,&signal,context,nullptr,slot,1,nullptr,meta);
 if(!connection)throw std::runtime_error(std::string("Could not connect the Qt signal: ")+signalName);
 qt<void(*)(void*)>(L"Qt6Core.dll","??1Connection@QMetaObject@@QEAA@XZ")(&connection);
}
