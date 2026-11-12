@echo off
setlocal
if /i not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
  echo Open "x64 Native Tools Command Prompt for VS" and run build.cmd.
  exit /b 1
)
pushd "%~dp0" || exit /b 1
if not exist "build\" mkdir "build" || goto :failed
pushd "build" || goto :failed

cl /nologo /std:c++17 /utf-8 /EHsc /O2 /MT /LD "..\src\native_bridge.cpp" /link /OUT:native_bridge_v032.dll user32.lib || goto :build_failed
cl /nologo /std:c++17 /utf-8 /EHsc /O2 /MT "..\src\launcher.cpp" /link /SUBSYSTEM:WINDOWS /OUT:TaperWarpTools.exe user32.lib || goto :build_failed

cl /nologo /std:c++17 /utf-8 /EHsc /MT "..\tests\test_compensation.cpp" || goto :build_failed
test_compensation.exe || goto :build_failed
cl /nologo /std:c++17 /utf-8 /EHsc /MT "..\tests\test_preview_svg.cpp" || goto :build_failed
test_preview_svg.exe || goto :build_failed
cl /nologo /std:c++17 /utf-8 /EHsc /MT "..\tests\test_lightburn_profile.cpp" || goto :build_failed
test_lightburn_profile.exe || goto :build_failed

popd
popd
exit /b 0

:build_failed
popd
:failed
popd
exit /b 1
