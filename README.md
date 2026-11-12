# Taper Warp Tools for LightBurn

An unofficial extension that pre-compensates vector designs for engraving on tapered drinkware. It adds two tools below **Taper Warp** in LightBurn's **Laser Tools** menu.

> This project aims to extend LightBurn's capabilities and make it easier to use, with respect for LightBurn and the work of its developers.

- **Taper Warp v2** uses a virtual reference rectangle so you can position the design within the engraving area.
- **Dual Taper Warp** models straight sections at the top and bottom and an adjustable curved transition between them.

Both tools preserve the design height, save their settings separately, and let you compare the compensated preview with the original. **Apply** creates an undo step in LightBurn. Text is converted to paths, and groups may be converted to individual paths. Select vector artwork to use either tool.

## Compatibility

The extension supports **LightBurn 2.1.04 on Windows x64** with executable SHA-256 `68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b`. The adapter uses internal functions from this specific build. The launcher verifies the executable hash before loading the extension into your existing LightBurn installation. See the [installation guide](INSTALL.md) for the portable ZIP.

## Build and package

Install Visual Studio 2022 or Build Tools with the x64 C++ tools and Windows SDK. Open **x64 Native Tools Command Prompt for VS** in the repository folder.

```bat
build.cmd
```

The script builds the launcher and DLL in `build/` and runs the compensation and preview tests without starting LightBurn. Building requires neither the Qt SDK nor any LightBurn files. To create the portable ZIP and its SHA-256 checksum, run this from the same prompt.

```bat
package.cmd
```

The package is written to `dist/`. Manual testing on a second installation is still pending.

See the [REA porting guide](docs/PORTING_WITH_REA.md) for updating the adapter when LightBurn releases a new build.

## Contact

For questions or feedback about this project, email [motar2kfan@gmail.com](mailto:motar2kfan@gmail.com).
