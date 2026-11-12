# Install Taper Warp Tools

1. Check that you have **LightBurn 2.1.04 for Windows x64**. The launcher supports the build with executable SHA-256 `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b`.
2. Extract the ZIP to a folder you can write to, such as Documents. Keep `TaperWarpTools.exe` and `native_bridge_v031.dll` in the same folder.
3. Save your work, close LightBurn, and run `TaperWarpTools.exe`. If the launcher cannot find LightBurn in the default location, it opens a file picker. If you have multiple installations, pass the path to the one you want to use.
4. In LightBurn, select vector artwork and open **Taper Warp v2** or **Dual Taper Warp** below **Taper Warp** in **Laser Tools**.

To specify a different LightBurn location, open Command Prompt in the extracted folder and run the launcher with the full path.

```bat
TaperWarpTools.exe "D:\Apps\LightBurn\LightBurn.exe"
```

The launcher creates a local copy of `LightBurn.exe` in `backups/` before loading the tools. The extension saves preferences in `taper-tools.ini`. If the menu entries do not appear, check `launcher-error.log` and `bridge.log` in the extracted folder.

To use LightBurn without the extension, close it and reopen it from its usual shortcut. To remove the extension, close LightBurn and delete the extracted folder after saving any backups or preferences you want to keep.
