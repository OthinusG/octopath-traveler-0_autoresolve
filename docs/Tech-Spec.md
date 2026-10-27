# OT0 Auto Resolve — Technical Specification

## Goal

Resolve eligible over-levelled ordinary random field encounters before battle, then grant rewards using verified OT0 game interfaces. Keep the project independent from the OT1 repository.

## Target

- Platform: Windows Steam, AppID 3014320, installed build 22012444.
- Engine: UE5.4 Shipping, confirmed by the running executable and UE4SS log.
- Loader: official UE4SS `experimental-latest`, non-dev archive `3.0.1-1181-g83330635` as resolved during project setup.
- The loader and shipping module have launched successfully; ordinary resolution, repeated native rewards, training and native level-up recovery have been observed. See VALIDATION.md for exact coverage.

## Intended behavior

- Only resolve ordinary random encounters caused by field movement.
- Use the highest active forward/back party level, excluding guests, with a configurable margin (default 1) against both area danger and every selected enemy level. Living active members and the native training roster receive rewards; dead active members do not.
- Grant only the rewards OT0 normally awards for the skipped encounter, including any verified level-up recovery behavior.
- F6 toggles the feature with a brief native notification, unless OT0 input/UI interfaces require another minimal approach.
- Boss, story, fixed, scripted and unclassified encounters remain normal battles.
- Do not write saves directly or simulate battle-only bonuses, stealing/capture, discovery, or other unverified battle effects.

## Evidence required before implementation

1. Resolve the precise UE5 version and confirm the selected experimental UE4SS build launches on the installed Steam build without changing saves or unrelated mods.
2. Extract/read OT0 encounter and reward assets from the user's installed game; keep raw assets and dumps local and excluded from the distributable source.
3. Verify encounter call flow, random encounter classification, party/level state, reward tables and native write interfaces.
4. Confirm guards can distinguish ordinary encounters from boss/story/fixed battles before any mutation.

## Current discovery status

- UE4SS `3.0.1-1181-g83330635` starts on the installed game and resolves UE 5.4 Shipping. The official `experimental-latest` non-dev asset listing was rechecked and still resolves to this archive/hash.
- A read-only UE4SS object scan found `QPEncountManager`, `QPBattleManager`, `KSSaveDataManager`, `QPSaveData`, and `RewardUtility`. The encounter manager exposes force/event/debug start methods and encounter map/draw-info getters. The battle manager exposes EXP/JP/money/drop result-ratio accessors.
- Native PDB signatures, selected function bodies and field runtime observations established the implementation contract below. Reflection alone was insufficient for the encounter and reward code.

## Acceptance criteria

- UE4SS and the mod load on the recorded game build without startup errors.
- At and above the configured threshold, a verified ordinary encounter resolves once and applies correct rewards/state updates.
- Below threshold, unknown, boss, story and fixed encounters proceed normally with no extra rewards.
- Repeated encounters and map transitions do not duplicate rewards, stall movement or alter protected battles.
- Missing/invalid interfaces fail closed before reward mutation.
- Existing saves, loaders, UE4SS settings and other mods remain unchanged.

## Native interception design

The installed game's PDB confirms encounter selection and reward application use non-reflected C++ functions. Use a small Windows x64 native library loaded by a UE4SS Lua bootstrap. Keep game-specific ABI declarations and supported-build offsets explicit; reject mismatched PE identity or call instructions before installing anything.

The verified field `AQPEncountManager::Tick` path checks field readiness, `IsEnableEncount(false)`, successful `UpdatePlayerStep`, and `IsEncount`; it then resets/sets up steps and calls `StandbyBattle(false, -1, false, ...)`. `StandbyBattle` calls `ChoiceEnemyData` before writing battle-mode fields or starting the battle. Scope eligibility to that exact field call, then inspect the native selected group in the choice call. A successfully resolved choice returns false to the remaining standby path, which leaves battle startup untouched. Other calls retain the original result. Never hook the generic event-battle start functions.

Implementation scope: native library and supported-build declarations; UE4SS Lua bootstrap and F6 configuration; focused eligibility/reward tests; portable build/package tools; bilingual README and exact runtime validation record. No static enemy catalog or extracted game assets are required: use verified native database accessors.

Verification must establish unchanged below-threshold/protected calls, native selected group identity, one reward application per field choice, EXP/JP/money/inventory deltas and level-up HP/SP recovery. Validate all reward interfaces and data before any writes, and disable further resolution after a partial settlement failure.

The game's saved TEXT_LANGUAGE value controls notifications, independently of voice and Unreal's general culture. The live native resource-language table establishes 0=Japanese, 1=English, 2=Simplified Chinese, 3=Traditional Chinese, 4=Korean. Use Chinese for 2/3 and English otherwise. No Lua culture lookup is required.

Training uses basis-point rates and native float multiplication by 0.0001 followed by ceiling. Ordinary passive/village/drop bonuses use their own native percentage conventions. Active meals and quest-specific drops retain normal combat until their special rules are verified.

## Risks

- UE4SS experimental builds can regress or have game-specific launch failures.
- OT0 encounter logic and settlement differ from OT1; incorrect hooks could skip protected content or corrupt progression.
- A successful game launch or UE4SS log alone does not validate encounter safety or reward correctness.

## Repository and homepage maintenance — 2026-10-27

Replace the inherited UE4SS project tree with the OT0 mod source, following the OT1 repository layout. Preserve upstream history and its original MIT notice; retain the OT0 native module, Lua bootstrap, tests, build tools, bilingual documentation and screenshots. Do not change gameplay behavior or checked-in shipping code.

Reuse the OT1 static bilingual documentation site and Pages workflow as the visual and deployment reference. Keep its layout, typography, colors and responsive behavior; replace game-specific copy, screenshots, versions, installation paths and repository links with verified OT0 information. The site has no backend or live data. Use the existing OT0 images and validate both languages at desktop and mobile widths before publication.

Tasks and acceptance:
- Remove UE4SS source, submodules, build tooling, inherited workflows and CNAME from the current tree without rewriting history.
- Add focused mod validation CI and Pages deployment, then update repository About, topics and homepage URL.
- Run Lua syntax/bootstrap, C++ logic, Windows native build and packaging checks; compare all mod/native files with the prior commit.
- Check site links, images, installation/build instructions and responsive screenshots. Verify the remote source tree, successful CI/Pages runs and public HTTP responses.

Risks: deleting inherited files must not remove OT0-owned native code or licensing notices. Public documentation and archives must exclude local diagnostics, game data, saves, binaries and host-specific details. Keep existing runtime evidence and limits unchanged.
