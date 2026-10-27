<img src="images/icon.png" alt="Octopath Traveler 0" width="96" height="96">

# OT0 Auto Resolve

[Homepage](https://othinusg.github.io/octopath-traveler-0_autoresolve/) · [简体中文](README.md)

Source release 0.1.0 for Octopath Traveler 0, Windows Steam AppID 3014320, build 22012444 and official UE4SS experimental build 3.0.1-1181-g83330635.

Resolve eligible over-levelled ordinary walking encounters before combat and grant rewards through game APIs. Boss, story, fixed, event and unclassified encounters remain normal battles. F6 toggles the mod and persists its setting.

- Highest active-party level must meet both area danger and selected enemy levels plus the configured margin. Forward/back active members count; guests do not.
- Native encounter selection preserves rare and first-group selections.
- Verified support, equipment, village and training reward rules apply to EXP, JP and leaves. Weighted drops respect native result and inventory limits.
- Knocked-out active members receive no EXP/JP. Actual level-ups refill that active or training character's HP/SP.
- Notifications follow the text language: Chinese for Chinese interfaces, English for everything else. Voice settings are independent.
- Active meals and quest-specific drops retain normal combat. No unverified battle-performance, stealing/capture or first-species effects are simulated.

![In-game screenshot: automatic encounter rewards in the English interface](images/ot0rs_en.png)

## Install

Install official [UE4SS experimental](https://github.com/UE4SS-RE/RE-UE4SS/releases/tag/experimental-latest) separately. The verified archive is `UE4SS_v3.0.1-1181-g83330635.zip`; the moving experimental release may contain a different build.

[Download the source ZIP](https://github.com/OthinusG/octopath-traveler-0_autoresolve/archive/refs/heads/main.zip), extract it, and run `python3 tools/package.py` with the [documented build tools](docs/DEVELOPMENT.md). The repository archive contains no prebuilt native DLL, so copying its mod folder alone is insufficient.

With the game closed, extract `dist/OT0AutoResolve-0.1.0.zip` and copy `OT0AutoResolve` into `Octopath_Traveler0/Binaries/Win64/ue4ss/Mods/`. Keep `enabled.txt`. If the loader uses `ue4ss/Mods/mods.txt`, enable `OT0AutoResolve : 1` and preserve other entries. Restart the game.

For upgrades, exit the game and replace only `OT0AutoResolve/Scripts`, including its DLL, preserving `config.ini`. Defaults under `[General]`: Enabled=1, LevelMargin=1, ShowUI=1. F6 is fixed. See the [Chinese user guide](mod/README.md).

## Test and package

Requires Lua for local bootstrap tests, a C++17 compiler for logic tests, and Python 3, Clang, LLVM dlltool and lld-link for Windows cross-compilation. The game's Lua runtime is supplied by UE4SS.

```sh
luac -p mod/OT0AutoResolve/Scripts/main.lua
lua tests/test_bootstrap.lua "$PWD"
clang++ -std=c++17 -Wall -Wextra -Werror tests/test_logic.cpp -o /tmp/ot0-test-logic
/tmp/ot0-test-logic
python3 tools/package.py
```

The packager produces `dist/OT0AutoResolve-0.1.0.zip`, `dist/OT0AutoResolve-0.1.0-source.zip` and `dist/SHA256SUMS.txt`. The mod package contains its own native DLL, but no loader binaries. Normal builds require no game extraction, PDBs, SDK dumps or UE4SS source.

The supported build passed repeat ordinary resolutions, active/training rewards, level-up HP/SP recovery, F6 persistence, below-threshold fallback and a shipping cold start. No positive item drop was observed in recorded acceptance samples; individual boss/story/fixed battles were not each played through. Other executable builds are not claimed compatible.

To uninstall, exit the game, remove only `ue4ss/Mods/OT0AutoResolve` and its own `mods.txt` entry, and retain the shared loader if other mods use it.

See [development notes](docs/DEVELOPMENT.md), [validation scope](docs/VALIDATION.md), and [notices](NOTICE.md). The upstream UE4SS MIT notice is retained under `LICENSES/UE4SS-MIT.txt`; it applies only to UE4SS. The OT0 mod source remains all rights reserved. This is an unofficial community mod, not affiliated with the publisher.
