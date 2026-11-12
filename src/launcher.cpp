// Quiet, explicit launcher. No service, autorun, binary patch or registry hook.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <commdlg.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cwctype>
#include "verified_image.h"
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"shell32.lib")
namespace fs=std::filesystem;
static const wchar_t* defaultExecutable=L"C:\\Program Files\\LightBurn\\LightBurn.exe";
static const auto& library=lightburn::bridgeName;
struct Handle {
 HANDLE value=nullptr;
 explicit Handle(HANDLE h=nullptr):value(h){}
 ~Handle(){if(value&&value!=INVALID_HANDLE_VALUE)CloseHandle(value);}
 Handle(const Handle&)=delete;Handle& operator=(const Handle&)=delete;
};
struct Module {std::wstring name,path;uintptr_t base;};
static std::wstring lower(std::wstring s){for(auto& ch:s)ch=std::towlower(ch);return s;}
static std::vector<Module> modules(DWORD pid) {
 Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid));
 if(snapshot.value==INVALID_HANDLE_VALUE) {
  DWORD error=GetLastError();if(error==ERROR_BAD_LENGTH||error==ERROR_PARTIAL_COPY)return {};
  throw std::runtime_error("Could not inspect LightBurn. Run both programs with the same privilege level.");
 }
 MODULEENTRY32W e{};e.dwSize=sizeof(e);std::vector<Module> result;
 if(Module32FirstW(snapshot.value,&e))do{result.push_back({lower(e.szModule),e.szExePath,reinterpret_cast<uintptr_t>(e.modBaseAddr)});}while(Module32NextW(snapshot.value,&e));
 return result;
}
static std::vector<DWORD> processes() {
 Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0));
 if(snapshot.value==INVALID_HANDLE_VALUE)throw std::runtime_error("Could not read the process list.");
 PROCESSENTRY32W e{};e.dwSize=sizeof(e);std::vector<DWORD> result;
 if(Process32FirstW(snapshot.value,&e))do{if(lower(e.szExeFile)==L"lightburn.exe")result.push_back(e.th32ProcessID);}while(Process32NextW(snapshot.value,&e));
 return result;
}
static std::vector<Module> waitForHost(DWORD pid) {
 for(int i=0;i<180;i++) {
  auto mods=modules(pid);bool host=false,qt=false;
  for(const auto& m:mods){if(m.name==L"lightburn.exe")host=true;if(m.name==L"qt6widgets.dll")qt=true;}
  if(host&&qt)return mods;Sleep(250);
 }
 throw std::runtime_error("LightBurn did not finish starting within 45 seconds.");
}
static bool alreadyLoaded(const std::vector<Module>& mods,const fs::path& dll) {
 for(const auto& m:mods)if(m.name.rfind(L"native_bridge",0)==0) {
  if(m.name!=library)throw std::runtime_error("Save your work and close LightBurn before loading this version of the tools.");
  if(lower(m.path)!=lower(dll.wstring()))throw std::runtime_error("Another copy of the tools is already loaded. Close LightBurn and run this launcher again.");
  return true;
 }
 return false;
}
static void load(DWORD pid,const fs::path& dll,const std::vector<Module>& mods, uintptr_t hostBase, const lightburn::Profile& profile) {
 auto loadLibrary=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
 HMODULE owner=nullptr;
 if(!loadLibrary||!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(loadLibrary),&owner))throw std::runtime_error("Could not locate LoadLibraryW.");
 wchar_t file[32768]{};GetModuleFileNameW(owner,file,32768);auto name=lower(fs::path(file).filename().wstring());uintptr_t remoteOwner=0;
 for(const auto& m:mods)if(m.name==name)remoteOwner=m.base;
 if(!remoteOwner)throw std::runtime_error("Could not locate the Windows library in the LightBurn process.");
 auto address=remoteOwner+reinterpret_cast<uintptr_t>(loadLibrary)-reinterpret_cast<uintptr_t>(owner);
 Handle process(OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ,FALSE,pid));
 if(!process.value)throw std::runtime_error("Windows blocked access to the LightBurn process. Run both programs with the same privilege level.");
 if(!lightburn::matchesMappedImage(process.value,hostBase,profile))
  throw std::runtime_error("The mapped LightBurn image does not match its build profile.");
 const auto text=dll.wstring();size_t bytes=(text.size()+1)*sizeof(wchar_t);
 void* memory=VirtualAllocEx(process.value,nullptr,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
 if(!memory)throw std::runtime_error("Could not allocate memory to load the tools.");
 try {
  SIZE_T written=0;
  if(!WriteProcessMemory(process.value,memory,text.c_str(),bytes,&written)||written!=bytes)throw std::runtime_error("Could not pass the library path to LightBurn.");
  Handle thread(CreateRemoteThread(process.value,nullptr,0,reinterpret_cast<LPTHREAD_START_ROUTINE>(address),memory,0,nullptr));
  if(!thread.value)throw std::runtime_error("Could not start loading the tools.");
  if(WaitForSingleObject(thread.value,10000)!=WAIT_OBJECT_0){memory=nullptr;throw std::runtime_error("The tools did not finish loading within 10 seconds.");}
  VirtualFreeEx(process.value,memory,0,MEM_RELEASE);memory=nullptr;
  if(!alreadyLoaded(modules(pid),dll))throw std::runtime_error("Windows could not load the tools library.");
 }catch(...){if(memory)VirtualFreeEx(process.value,memory,0,MEM_RELEASE);throw;}
}
static std::string read(const fs::path& path) {std::ifstream in(path);return {std::istreambuf_iterator<char>(in),{}};}

static fs::path selectExecutable() {
 int count=0;
 auto args=CommandLineToArgvW(GetCommandLineW(),&count);
 if(!args)throw std::runtime_error("Could not read the launcher arguments.");
 fs::path explicitPath=count==2?args[1]:L"";
 LocalFree(args);
 if(count==2&&!explicitPath.empty())return fs::absolute(explicitPath);
 if(count!=1)throw std::runtime_error("Pass one path to LightBurn.exe as the launcher argument.");
 if(fs::is_regular_file(defaultExecutable))return defaultExecutable;

 wchar_t file[32768]{};
 OPENFILENAMEW dialog{};
 dialog.lStructSize=sizeof(dialog);
 dialog.lpstrFilter=L"LightBurn (LightBurn.exe)\0LightBurn.exe\0";
 dialog.lpstrFile=file;
 dialog.nMaxFile=static_cast<DWORD>(sizeof(file)/sizeof(file[0]));
 dialog.lpstrTitle=L"Select LightBurn.exe";
 dialog.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(GetOpenFileNameW(&dialog))return file;
 if(CommDlgExtendedError())throw std::runtime_error("Could not open the file picker for LightBurn.exe.");
 return {};
}

static void run(const fs::path& home, const fs::path& executable) {
 if(lower(executable.filename().wstring())!=L"lightburn.exe")
  throw std::runtime_error("Select the LightBurn.exe file.");
 const lightburn::ImageFile image(executable);
 const auto* profile=lightburn::findProfile(image.sha256());
 if(!profile)throw std::runtime_error("Unsupported LightBurn executable SHA-256. Select a supported Windows x64 build of LightBurn 2.1.04 or 2.2.01.");
 auto dll=home/library;if(!fs::is_regular_file(dll))throw std::runtime_error("Place native_bridge_v032.dll in the same folder as the launcher.");
 auto backup=home/L"backups"/std::string(profile->sha256,8)/L"LightBurn.exe";fs::create_directories(backup.parent_path());
 if(!fs::exists(backup))fs::copy_file(executable,backup);
 if(lightburn::ImageFile(backup).sha256()!=profile->sha256)
  throw std::runtime_error("The executable backup does not match the original.");
 auto running=processes();DWORD pid=0;
 if(running.size()>1)throw std::runtime_error("Multiple LightBurn instances are running. Close the extra instances and run the launcher again.");
 if(running.size()==1)pid=running.front();
 else {
  STARTUPINFOW si{};si.cb=sizeof(si);PROCESS_INFORMATION pi{};
  std::wstring command=L"\""+executable.wstring()+L"\"";
  if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,executable.parent_path().c_str(),&si,&pi))throw std::runtime_error("Could not start LightBurn.");
  pid=pi.dwProcessId;CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
 }
 auto mods=waitForHost(pid);uintptr_t hostBase=0;
 for(const auto& m:mods)if(m.name==L"lightburn.exe"){
  if(!fs::equivalent(m.path,executable)||lightburn::ImageFile(m.path).sha256()!=profile->sha256)
   throw std::runtime_error("A different LightBurn installation is running. Close it and run the launcher again.");
  hostBase=m.base;
 }
 if(!hostBase)throw std::runtime_error("Could not verify the running LightBurn executable.");
 const auto failure=home/(L"startup-"+std::to_wstring(pid)+lightburn::startupErrorSuffix);
 const auto success=std::to_string(pid)+" "+lightburn::packageVersion;
 if(!alreadyLoaded(mods,dll)) {
  if(fs::exists(failure))fs::remove(failure);
  if(read(home/L"ready.txt")==success)fs::remove(home/L"ready.txt");
  load(pid,dll,mods,hostBase,*profile);
 }
 for(int i=0;i<280;i++) {
  if(read(home/L"ready.txt")==success)return;
  if(fs::exists(failure))throw std::runtime_error(read(failure));
  Sleep(250);
 }
 throw std::runtime_error("The tools menu did not report that it was ready. Check bridge.log and any open LightBurn startup dialogs.");
}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
 wchar_t file[32768]{};GetModuleFileNameW(nullptr,file,32768);auto home=fs::path(file).parent_path();
 try {
  const auto executable=selectExecutable();
  if(!executable.empty())run(home,executable);
  return 0;
 }
 catch(const std::exception& e) {
  std::ofstream(home/L"launcher-error.log")<<e.what()<<"\n";
  MessageBoxA(nullptr,e.what(),"LightBurn - Taper Tools",MB_OK|MB_ICONINFORMATION);return 1;
 }
}
