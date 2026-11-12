# Taper Warp Tools compatibility review for LightBurn 2.2.01

Taper Warp Tools 0.3.2 includes a profile for the installed Windows x64 executable with SHA-256 `1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b`. Windows installer registration reports 2.2.01. The executable and PDB match on CodeView GUID `990ae889-f6c8-43a2-b6f5-b8c9a7f750ed`, age 1.

The baseline executable is 2.1.04 with SHA-256 `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b` and matching PDB GUID `2b1ad6e2-f984-4f8a-8538-f1962ab179fb`, age 1.

## Verified adapter addresses

RVAs relative to the loaded executable, derived by matching the 2.1.04 and 2.2.01 PDB symbols or inspecting the corresponding instructions in the native Taper Warp.

| Binding | 2.1.04 RVA | 2.2.01 RVA |
| --- | --- | --- |
| Shape list destructor | `0x1f3b0` | `0xedff0` |
| MinList::AddNode | `0x20570` | `0x118f20` |
| SendLaserEvent | `0x29480` | `0x1228f0` |
| UndoAddNewShapes constructor | `0x918a0` | `0x1bdae0` |
| UndoChangeSelection constructor | `0x92b50` | `0x1be910` |
| UndoDeleteShapes constructor | `0x97730` | `0x1c2240` |
| UndoGroup constructor | `0x99530` | `0x1c3c70` |
| UndoSystem::AddUndo | `0xa06d0` | `0x1cabc0` |
| UndoGroup::Undo | `0xae370` | `0x1d88c0` |
| PathShape::BuildPainterPath | `0x2e28c0` | `0x5ddbd0` |
| PathShape::ComputeBounds | `0x2eb8d0` | `0x5e68d0` |
| Shape::Flatten | `0x2f36f0` | `0x5ef750` |
| PathShape::LimitPrimLength | `0x3043b0` | `0x600b60` |
| TaperWarpDialog::Update | `0x6834c0` | `0xaf5940` |
| operator new | `0xcce530` | `0x11d0890` |
| Editor context pointer | `0x1948bb0` | `0x1f73f50` |
| Coordinate mode pointer | `0x1948e28` | `0x1f741d8` |
| Display unit multiplier | `0x194935c` | `0x1f7470c` |
| Display unit suffix | `0x1949368` | `0x1f74718` |
| Undo manager | `0x19493c8` | `0x1f74778` |

## Static checks

- **Identity and guard.** REA opened both files as PE x86-64 and reported the full hashes. Its objdump evidence is `ev_a8b1ea4808b61cbe49b98a93a30f90b8b0d5509eefbafeff15e2919c2c69b792` for 2.2.01 and `ev_4d79e36bdce5385ff7013f73d34a6f236e4f74bb1fdaa75d50a02fb8c126daba` for 2.1.04. REA's deep PE provider was unavailable. The address and ABI checks used the matching PDBs, targeted local objdump and PE section reads. The Update guard is `48 8b c4 55 53 56 57 41 54 41 55` for 2.2.01 and `48 8b c4 48 89 58 10 48 89 70 18` for 2.1.04.
- **Menu and selection.** The executable contains `actionTaper_Warp`. The MainWindow metacall at RVA `0xa09d31` takes the editor selection at context +`0xe8`. SetShapes at `0xaedd40` copies the three pointer-sized fields of the Qt list and increments the 32-bit shared refcount. Flatten at `0x5ef760` calls virtual slot 1 for type, traverses group type 10 from +`0xe8` through sibling links at +`0x08`, skips bitmap type 7 and uses deleting-destructor slot 0. The PDB-backed bitmap, array and group type returns match the baseline.
- **Geometry and units.** The matching decorated symbols identify Flatten's output list and float tolerance, LimitPrimLength's float argument and ComputeBounds' member signature. LimitPrimLength and BuildPainterPath use the vertex list at +`0xf8` with a 28-byte stride. BuildPainterPath uses the caller-provided return buffer in RDX. Update reads the orientation byte at coordinate mode +3 and the unit multiplier as a 32-bit float. The native dialog constructor passes the unit suffix address as a QString reference.
- **Apply, Undo and Redo.** Update at `0xaf5cd7` through `0xaf5dee` uses the host allocator for a 0x50-byte UndoGroup and 0x40, 0x48 and 0x68-byte add, selection and delete nodes, respectively. The group constructor matches the inlined setup in Update, with the name at +`0x18` and the node list at +`0x30`. The native call sites confirm the constructor arguments and node order. The metacall at `0xa09d5a` submits the group to the undo manager on accept. Its cancel branch calls the group's Undo virtual before deleting it. Update emits document-change event 1001.
- **Qt.** The installed Qt6Core, Qt6Gui and Qt6Widgets DLLs report 6.11.2. All 90 decorated bindings and `imageformats/qsvg.dll` are present. The QObject-derived primary vtables inspected for the dialog, widgets, layouts and settings resolve deleting destructors at slot 3. Their destruction code is consistent with the bridge's object storage. QString and QByteArray access use the 24-byte Qt array-data layout. The QAction, QMenu, QAbstractButton and QDialogButtonBox metacalls use the expected member-function lookup, and Qt's callback path uses the slot reference count and implementation pointer. Plugin loading and object lifetimes in the editor require manual verification.

## Verification status

- **Standalone:** Windows x64 build, compensation tests, SVG preview tests, both exact executable hashes, unknown-hash rejection and mapped guard rejection passed.
- **Manual:** 2.2.01 remains a candidate until the user verifies menu placement, preview, Cancel, Apply, Undo and Redo for both tools in the editor, including a non-default project page. See the [manual test steps](../INSTALL.md#test-both-tools).
