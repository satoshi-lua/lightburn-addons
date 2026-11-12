# Porting Taper Warp Tools with REA

Use this workflow to assess or add support for a new LightBurn build. The launcher and native adapter currently support one exact Windows x64 build. See the [LightBurn 2.2.01 investigation](lightburn-2.2.01.md) for an example of candidate addresses recovered from a real update.

## 1. Identify the target and baseline

1. Identify the `LightBurn.exe` selected by the user. Record its installed product version, PE architecture, SHA-256, and the versions of `Qt6Core.dll`, `Qt6Gui.dll`, and `Qt6Widgets.dll` from the same installation. The executable's version resource can be blank. Use the installer registration or other local package metadata in that case.
2. Use the last supported binary as the baseline. Verify its SHA-256 against [the launcher](../src/launcher.cpp). A local backup is suitable. For each build with a PDB, match the executable's PE CodeView RSDS GUID and age against the PDB before trusting symbol addresses.
3. Keep the analysis and any backup outside the repository. If a backup of the target is missing, copy the executable to a local folder named for its hash and verify the copy before any manual integration test. Work from copies when using a provider that produces analysis files. Record paths privately and leave the installed files intact.

```powershell
$lightBurnExe = 'C:\Program Files\LightBurn\LightBurn.exe'
Get-FileHash -LiteralPath $lightBurnExe -Algorithm SHA256
(Get-Item -LiteralPath $lightBurnExe).VersionInfo
```

## 2. Recover the native interfaces in REA

Read the installed `reverse-engineer-anything` skill when available and discover the actual REA MCP tool schemas in the session. Open the baseline and target in turn with `open_binary`. Confirm the reported PE architecture and SHA-256 each time. Retain the evidence for the first target before switching. Use the following tools when the active provider supports them.

| Task | REA call |
| --- | --- |
| Bind the installed executable | `open_binary` with `path` set to the absolute executable path |
| Locate the menu entry and related procedures | `search_strings` for `actionTaper_Warp`, then `search_procedures` with a literal pattern such as `TaperWarpDialog` |
| Inspect the code that consumes a symbol | `analyze_function` with the resolved `procedure`; use procedure assembly or cross-references for calling details |
| Verify a signature used by the guard | `read_bytes` with the provider address and exact length, or read those bytes from the local PE section |
| Finish the static session | `close_binary` |

Start with the Qt action and its MainWindow metacall, then follow the calls into the native Taper Warp dialog. Match decorated names across the verified PDBs when available. Corroborate the calls and data references in disassembly. If REA can identify the PE but has no native analysis provider, use a matching PDB with a local symbol reader and targeted `objdump` or `dumpbin` disassembly. When symbols are absent, trace the action, call sites, and data references in the disassembler. Record missing evidence instead of accepting the first byte-pattern match.

Convert virtual addresses to RVAs using each executable's own image base. Field offsets within objects are separate from RVAs. For every value the bridge uses, record the symbol or call site, RVA or field offset, signature or layout, and evidence reference. Verify these groups before implementation.

| Area | What to confirm |
| --- | --- |
| Menu and selection | QAction object name, parent menu, editor context pointer and selection list layout, supported shape type IDs, virtual methods, group traversal and child ownership |
| Geometry | Flattening, subdivision, bounds calculation, painter path return convention, vertex list offset and stride, and the orientation flag that maps editor coordinates to preview coordinates |
| Units | Unit multiplier, unit suffix, and their types. Cross-check against the native dialog |
| Apply, Undo, and Redo | Host allocator and ownership, undo constructors and allocation sizes, group fields and list insertion, manager submission, rollback, and the document-change event |
| Qt | Exact decorated exports in the expected DLLs, signals and metacalls, callback ABI, object storage and destruction, and the SVG image plugin |

Use [native_bridge.cpp](../src/native_bridge.cpp), [native_dialog.h](../src/native_dialog.h), [native_signal.h](../src/native_signal.h), and [native_settings.h](../src/native_settings.h) as the inventory. Export names alone do not establish that an object layout or callback ABI still matches.

## 3. Add and package a build profile

When implementation is in scope, add a profile for the exact target SHA-256. The current source contains inline constants for the original build. Move the version-dependent values into named profiles as needed, keeping shared dialog and geometry logic common. Keep the known profile intact and make the launcher and bridge agree on the selected build and binary guard. Reject unknown executables before loading the DLL, and abort bridge initialization before using internal addresses if the selected profile's guard fails.

Keep the launcher, DLL filename, readiness protocol, package contents, and compatibility docs in sync. Give a changed DLL a distinct filename so a previous copy can remain loaded until the user exits LightBurn. The backup folder must correspond to the verified executable. Keep compatibility notes for each added build in `docs/`, with the executable hash, evidence, and separate static and manual verification status.

From an x64 Visual Studio Developer Command Prompt, run [build.cmd](../build.cmd) for the compiler and standalone tests. Run [package.cmd](../package.cmd) to build the ZIP, then inspect its file list and verify the checksum. Check that the old profile still selects the old build and that an unknown hash is rejected. Keep proprietary inputs and diagnostics out of commits and the ZIP.

## 4. Hand off the manual test

Give the user the new ZIP and the exact build it targets. Ask them to save their work, close LightBurn, extract the ZIP to a fresh folder, and launch it themselves. Use a disposable project with a known vector design. The user checks that both entries appear below **Taper Warp**, that the preview is upright, that the units and original comparison are correct, and that **Cancel** leaves the design intact. They then use **Apply**, **Undo**, and **Redo** for both tools and check the selection, geometry, and layer assignments. On builds with multiple project pages, include a test on a non-default page.

If a check fails, record the exact step and ask the user for the relevant excerpt from `launcher-error.log` or `bridge.log`. Report the build as a candidate until the manual checks pass. For assessment-only work, stop after recording the findings and the remaining work.
