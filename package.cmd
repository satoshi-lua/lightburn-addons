@echo off
setlocal
pushd "%~dp0" || exit /b 1
call build.cmd || goto :failed
if not exist "dist\" mkdir "dist" || goto :failed
set "TAPER_WARP_ARCHIVE=dist/TaperWarpTools-0.3.2-win-x64.zip"
tar.exe -a -cf "%TAPER_WARP_ARCHIVE%" INSTALL.md -C build TaperWarpTools.exe native_bridge_v032.dll || goto :failed
certutil.exe -hashfile "%TAPER_WARP_ARCHIVE%" SHA256 > "%TAPER_WARP_ARCHIVE%.sha256.txt" || goto :failed
echo %TAPER_WARP_ARCHIVE%
popd
exit /b 0

:failed
popd
exit /b 1
