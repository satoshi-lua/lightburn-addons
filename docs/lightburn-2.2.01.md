# Taper Warp Tools compatibility review for LightBurn 2.2.01

Static inspection only. The installed Windows x64 executable has SHA-256 `1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b`. Its PE CodeView record and installed PDB match, GUID `990ae889-f6c8-43a2-b6f5-b8c9a7f750ed`, age 1. Windows identifies the installation as 2.2.01.

## Candidate adapter addresses

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

The native menu handler still loads the selection at editor context +0xe8 and passes it to SetShapes. PathShape still uses the vertex list at +0xf8 with a 28-byte stride. Update still reads the orientation flag at +3, creates the same three undo nodes and emits event 1001. The menu handler still submits the undo group to UndoSystem::AddUndo. The action name remains `actionTaper_Warp`.

The new installation ships Qt 6.11.2. All 90 decorated names referenced by the bridge and the Qt SVG image plugin are present. Static signal metacalls are consistent with the callback bridge, including QMenu's member-pointer adjustment check. Export availability and these checks do not verify all object layouts or in-process behavior.

The port needs a separate executable profile with the exact SHA-256, updated addresses and binary guard. Remaining checks include group traversal and shape type IDs, object layouts, Qt allocation and cleanup, and a user-run test of both dialogs, Apply, Undo and Redo. The repository, existing package and LightBurn installation were left unchanged during this review.
