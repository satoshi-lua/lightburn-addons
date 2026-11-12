# Taper Warp Tools for LightBurn

An unofficial extension that pre-compensates vector designs for engraving on tapered drinkware. It adds two tools below **Taper Warp** in LightBurn's **Laser Tools** menu.

> This project aims to extend LightBurn's capabilities and make it easier to use, with respect for LightBurn and the work of its developers.

- **Taper Warp v2** uses a virtual reference rectangle so you can position the design within the engraving area.
- **Dual Taper Warp** models straight sections at the top and bottom and an adjustable curved transition between them.

Both tools preserve the design height, save their settings separately, and let you compare the compensated preview with the original. **Apply** creates an undo step in LightBurn. Text is converted to paths, and groups may be converted to individual paths. Select vector artwork to use either tool.

## Screenshots

**Dual Taper Warp** shows the compensated design next to the vessel profile, with the upper straight section, transition, and lower straight section marked.

![Dual Taper Warp dialog with a compensated vector and a dimensioned three-zone vessel profile](docs/screenshots/dual-taper-warp.png)

**Taper Warp v2** shows the compensated design inside the virtual reference rectangle.

![Taper Warp v2 dialog with the design position slider, reference rectangle, and tapered vessel profile](docs/screenshots/taper-warp-v2-dialog.png)

Both tools appear under **Taper Warp** in the **Laser Tools** menu.

![LightBurn Laser Tools menu showing Taper Warp v2 and Dual Taper Warp below Taper Warp](docs/screenshots/laser-tools-menu.png)

## Compatibility

The Windows x64 launcher verifies the full executable SHA-256 and checks the mapped image before loading the extension.

| LightBurn | Executable SHA-256 | Verification |
| --- | --- | --- |
| 2.1.04 | `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b` | Existing adapter carried forward |
| 2.2.01 | `1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b` | Static checks and standalone tests passed. Manual editor checks pending. |

See the [installation guide](INSTALL.md) for the portable ZIP and manual test steps.

## Build and package

Install Visual Studio 2022 or Build Tools with the x64 C++ tools and Windows SDK. Open **x64 Native Tools Command Prompt for VS** in the repository folder.

```bat
build.cmd
```

The script builds the launcher and DLL in `build/` and runs the compensation, preview and build-profile tests without starting LightBurn. Building requires neither the Qt SDK nor any LightBurn files. To create the portable ZIP and its SHA-256 checksum, run this from the same prompt.

```bat
package.cmd
```

The package is written to `dist/`.

See the [REA porting guide](docs/PORTING_WITH_REA.md) for updating the adapter when LightBurn releases a new build.

## Contact

For questions or feedback about this project, email [motar2kfan@gmail.com](mailto:motar2kfan@gmail.com).
