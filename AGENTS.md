# Updating LightBurn compatibility

Use [the REA porting guide](docs/PORTING_WITH_REA.md) when a LightBurn update changes the executable, its Qt libraries, or the internal interfaces used by this extension. Start with the installed executable's SHA-256 and the last supported build. Reuse matching static evidence where available.

Keep each supported build bound to its exact executable hash. Check the native function signatures, object layouts, and Qt bindings before enabling a new build. Preserve the existing tool behavior and the previous build's support unless the user requests otherwise.

The user performs all interactive LightBurn checks. Agents may inspect files, compile the extension, and run standalone tests. Leave launching LightBurn, loading the DLL into LightBurn, and trying the tools in its editor to the user.

Keep LightBurn binaries, PDBs, raw disassembly, REA evidence bundles, and local runtime files outside Git and the portable ZIP. Record compatibility findings and their verification status in the repository as described in the guide.
