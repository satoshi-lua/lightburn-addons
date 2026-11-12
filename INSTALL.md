# Install Taper Warp Tools

1. Check that you have one of the exact **Windows x64** builds listed below.
2. Extract the ZIP to a fresh folder you can write to, such as a folder in Documents. Keep `TaperWarpTools.exe` and `native_bridge_v032.dll` together.
3. Save your work, close LightBurn, and run `TaperWarpTools.exe`. If the launcher cannot find LightBurn in the default location, it opens a file picker. If you have multiple installations, pass the path to the one you want to use.
4. In LightBurn, select vector artwork and open **Taper Warp v2** or **Dual Taper Warp** below **Taper Warp** in **Laser Tools**.

| LightBurn | Executable SHA-256 |
| --- | --- |
| 2.1.04 | `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b` |
| 2.2.01 | `1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b` |

To specify a different LightBurn location, open Command Prompt in the extracted folder and run the launcher with the full path.

```bat
TaperWarpTools.exe "D:\Apps\LightBurn\LightBurn.exe"
```

The launcher creates and verifies a local copy of `LightBurn.exe` in a hash-specific folder under `backups/` before loading the tools. The extension saves preferences in `taper-tools.ini`. If a check fails, record the exact step and check `launcher-error.log` and `bridge.log` in the extracted folder.

## Test both tools

1. Create a disposable project with asymmetric vector artwork on two layers. Include a group and note the artwork's dimensions, position and layers.
2. Select the artwork. Verify that both entries appear below **Taper Warp**. Open **Taper Warp v2**, set different top and bottom diameters, and use a reference height larger than the artwork. Move the design vertically within the reference area. Check the units, upright orientation and original comparison. Click **Cancel** and verify the project is intact.
3. Reopen **Taper Warp v2** and click **Apply**. Verify the warped outlines, unchanged height, selection and layers. Use **Undo** once to restore the original artwork, then **Redo** once to restore the warped result. Verify the geometry, layers and selection after each step.
4. Undo the warp to start from the original artwork. Repeat **Cancel**, **Apply**, **Undo** and **Redo** with **Dual Taper Warp**. Use nonzero straight sections and change the curve setting so you can identify the transition in the preview and the result.
5. On a build with project pages, repeat **Apply**, **Undo** and **Redo** with each tool on a non-default page. Check that the edit stays on that page.

To use LightBurn without the extension, close it and reopen it from its usual shortcut. To remove the extension, close LightBurn and delete the extracted folder after saving any backups or preferences you want to keep.
