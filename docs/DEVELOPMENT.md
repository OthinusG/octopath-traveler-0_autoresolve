# Development

## Layout

- `native/`: freestanding Windows x64 callsite hooks and supported-build fingerprint.
- `mod/OT0AutoResolve/`: UE4SS Lua bootstrap, configuration and default enable marker.
- `tests/`: Lua bootstrap and portable native-logic checks.
- `tools/build_native.py`: builds a shipping DLL; development helpers are opt-in and must not be distributed.
- `tools/package.py`: builds the mod-only and source archives from explicit file lists.

## Requirements

Python 3, Lua, Clang targeting Windows x64, LLVM `llvm-dlltool`, and `lld-link`. Game files, UE4SS SDK, local PDBs and extracted assets are not needed for the normal build or checks.

## Commands

```sh
python3 tools/build_native.py
clang++ -std=c++17 -Wall -Wextra -Werror tests/test_logic.cpp -o /tmp/ot0-test-logic
/tmp/ot0-test-logic
luac -p mod/OT0AutoResolve/Scripts/main.lua
lua tests/test_bootstrap.lua "$PWD"
python3 tools/package.py
```

The executable hooks are specific to Steam build 22012444. Do not update offsets or fingerprints without re-verifying the call flow and protected encounter behavior in the target build. Never include `.local/`, game data, saves, PDB/SDK dumps, UE4SS binaries, or runtime logs in a public change.
