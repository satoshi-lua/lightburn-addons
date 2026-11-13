# Install Taper Warp Tools

1. Check that you have one of the exact **Windows x64** builds listed below.
2. Extract the ZIP to a fresh folder you can write to, such as a folder in Documents. Keep `TaperWarpTools.exe` and `native_bridge_v0312.dll` together.
3. Save your work, close LightBurn, and run `TaperWarpTools.exe`. If the launcher cannot find LightBurn in the default location, it opens a file picker. If you have multiple installations, pass the path to the one you want to use.
4. In LightBurn, select vector artwork and open **Taper Warp v2** or **Dual Taper Warp** below **Taper Warp** in **Laser Tools**.

| LightBurn | Executable SHA-256 |
| --- | --- |
| 2.1.04 | `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b` |
| 2.2.01 | `1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b` |

Manual verification of 2.2.01 is pending.

To specify a different LightBurn location, open Command Prompt in the extracted folder and run the launcher with the full path.

```bat
TaperWarpTools.exe "D:\Apps\LightBurn\LightBurn.exe"
```

The launcher creates and verifies a local copy of `LightBurn.exe` in a hash-specific folder under `backups/` before loading the tools. The extension saves preferences in `taper-tools.ini`. If a check fails, record the exact step and check `launcher-error.log` and `bridge.log` in the extracted folder.

## Test both tools

### Saved profiles

Set the vessel measurements and use **Save as…** under **Geometry profile**. **Update** saves the current measurements to the selected profile. Set **Distance from top** for each design.

To save a placement, first move the selected artwork to the desired engraving spot in the editor. Open either tool, choose **Placement rotation**, and use **Save as…** under **Engraving position**. **Update** saves the current artwork center and selected rotation to the selected profile. When **Place at saved position** is enabled, **Apply** warps the design in its original orientation, then rotates and centers it at the saved point. The preview shows the warp in the vessel's upright reference. Profiles are stored in the local `taper-tools.ini`. Position profiles from 0.3.3 load with a rotation of 0°.

For a manual check, save a position in one tool, then select upright artwork in a different spot and use the saved position from the other tool. With **90° clockwise**, verify that the top of the design ends up on the right and its center matches the saved location. Repeat with **90° counterclockwise** and check that the top ends up on the left. **Undo** should restore the original geometry, orientation, and location in one step. **Redo** should restore the warped artwork at the saved position and angle.

### Place a geometry reference

1. Select artwork and open either warp tool. Load the vessel geometry and choose **Placement rotation**.
2. Click **Place Geometry Reference…**. The warp dialog closes temporarily and a floating panel opens. LightBurn selects a real rectangle on tool layer **T1**, sized to the current reference height by 30 mm and oriented for the selected rotation.
3. Drag the selected rectangle or rotate it using the canvas controls. Click **Confirm Position** to save its final center and rotation, then name the position profile. The tool supports quarter turns in either direction and a 180° turn. **Cancel** removes the rectangle and returns to the warp dialog. The native placement edits appear in LightBurn's undo history.
   If **T1** was hidden, the tool shows it during placement and restores its original visibility when you confirm or cancel.
4. To check the offset, set **Distance from top** to a known value and apply the saved profile. The design's top should sit that distance from the reference area's top end after the selected rotation. With **90° clockwise**, the top end is on the right.

### Warp and undo checks

1. Create a disposable project with asymmetric vector artwork on two layers. Include a group and note the artwork's dimensions, position and layers.
2. Select the artwork. Verify that both entries appear below **Taper Warp**. Open **Taper Warp v2**, set different top and bottom diameters, and use a reference height larger than the artwork. Move the design vertically within the reference area. Check the units, upright orientation and original comparison. Click **Cancel** and verify the project is intact.
3. Reopen **Taper Warp v2** and click **Apply**. Verify the warped outlines, unchanged height, selection and layers. Use **Undo** once to restore the original artwork, then **Redo** once to restore the warped result. Verify the geometry, layers and selection after each step.
4. Undo the warp to start from the original artwork. Repeat **Cancel**, **Apply**, **Undo** and **Redo** with **Dual Taper Warp**. Use nonzero straight sections and change the curve setting so you can identify the transition in the preview and the result.
5. On a build with project pages, repeat **Apply**, **Undo** and **Redo** with each tool on a non-default page. Check that the edit stays on that page.

To use LightBurn without the extension, close it and reopen it from its usual shortcut. To remove the extension, close LightBurn and delete the extracted folder after saving any backups or preferences you want to keep.
