# Project Rules

## Scope
Maintain the Octopath Traveler 0 ordinary-encounter auto-resolve mod. Read README.md, MEMORY.md and Tech-Spec.md before implementation changes.

## Target and safety
- Windows Steam AppID 3014320, installed build 22012444; official UE4SS experimental build `3.0.1-1181-g83330635`.
- Only verified ordinary walking encounters may resolve. Boss, story, fixed, event and unknown encounters must remain native battles.
- Never edit saves directly or overwrite existing loaders, settings or other mods.
- Do not commit game assets, SDK/PDB dumps, saves, UE4SS binaries, local diagnostics, or host-specific connection details.

## Checks
Run Lua syntax/bootstrap checks, C++ logic tests, native Windows build and `python3 tools/package.py` for shipping changes. Runtime claims must match VALIDATION.md and exact game/UE4SS builds.
