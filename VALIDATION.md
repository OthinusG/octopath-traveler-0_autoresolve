# Runtime validation

## Target

- Windows Steam AppID 3014320, build 22012444.
- UE5.4 Shipping, executable SHA256 `2f3935ac7de3e6184e3b0a5ec47c5ad4214f9cf38e883764f2c84bd691083aeb`.
- Official UE4SS `UE4SS_v3.0.1-1181-g83330635.zip`, SHA256 `b9a166a756605b21058a64cde93bcaab1a855dba3553e5c7e6ce0fa8fdfad7b9`.

## Observed

- UE4SS startup and native executable/callsite fingerprint checks succeeded.
- Disabled ordinary encounter: group 99509 entered normal combat. No mod rewards were granted. Native reward plan was EXP 36, JP 27, leaves 665. Escaped using the game's normal menu.
- Enabled ordinary encounter: group 99513 resolved on the field without battle startup. All eight living active members gained EXP 59 and JP 40. Leaves changed from 154112 to 155093 (+981).
- Native level-up recovery reached level 30 with HP 1661 and SP 138; the ordinary menu showed both at their new maxima.
- A clearly labeled English preview appeared through the game's normal notification UI; the preview granted no rewards.
- Shipping-only Lua startup and enabled.txt discovery succeeded. F6 toggled persisted configuration. Two consecutive normal-interval encounters (group 99509) each granted active EXP 36/JP 27, training EXP 26/JP 22 and leaves 665. The second encounter again verified native HP/SP restoration on level-up.
- Raising the margin to 4 with highest party level 30 and area danger 27 produced normal combat (group 99510), with no added mod rewards.
- The game's native resource-language table was read to verify Japanese/default, EN-US, ZH-CN, ZH-TW and KO-KR order. The implementation now reads saved TEXT_LANGUAGE directly; the earlier generic Unreal culture lookup was incorrect for this game's Chinese interface.
- The final language revision passed a fresh UE4SS startup on 2026-10-26. F6 displayed the Simplified Chinese title and disabled message correctly in the loaded town. Installed native/Lua file hashes equal the source-built files.
- The current level-31 save resolved three further normal field groups without combat: 99502 (EXP 45/JP 27/leaves 663), 99505 (97/40/983), and 99506 (75/40/993). Training deltas were respectively 32/22, 68/32 and 53/32; a training member's native level-up restored HP/SP as well. No positive item roll occurred in these samples.
- The first development test exposed incorrect training conversion. Native code uses 0.0001, and the implementation now matches it. Regression cases: EXP 59 at 7000 basis points gives 42; JP 40 at 8000 gives 32. The excess training gains from the first test were not saved: all eight existing save-folder files retained their baseline hashes before returning to title and closing the game.

## Automated checks

- C++ margin boundaries, protected-group flags, weighted drop rounding and training rounding/overflow checks pass.
- Duplicate item totals use the native battle-result limit of 99, then remaining inventory capacity; focused checks cover both limits and a full inventory.
- Lua bootstrap tests cover scheduling, native exports and F6; Lua syntax passes. C++ checks cover Simplified/Traditional Chinese and English fallback for other or unknown language values.
- Windows x64 cross-compilation passes with warnings treated as errors. Shipping imports only Kernel32 and exports only initialization and toggle functions.
- Mod-only/source ZIP CRC checks, shipping binary equality and development-helper exclusions pass.
- A fresh source ZIP extraction in a directory containing spaces compiled successfully, produced a byte-identical shipping DLL, and passed C++/Lua checks.
- Final shipping DLL SHA256: `872a7c08c56cafa6e47a5bda8ae6ebb37257f4b6dc67c11c3753d08be40a5891`. It passed a fresh game startup with module retention enabled. Installed DLL, Lua, INI and enabled marker match the source/build files. Development probe folder/load entry and temporary launch tasks were removed.
- After final checks, all five manual save slots matched the current user's backup. The game's autosave and system-save files changed during normal map travel/startup; no save files were directly edited or restored.

## Outstanding acceptance

A positive item-drop delta and individual boss/story/fixed-battle playthroughs have not yet been recorded. Their guards are based on the verified native call flow and group/enemy checks. No compatibility claim is made for other executable builds. Active cooked meals and quest-specific drops deliberately retain normal combat.

Lua reload stress testing was not performed; the existing UE4SS reload settings were preserved. Native module retention prevents Lua's library-handle cleanup from unloading installed hook code.
