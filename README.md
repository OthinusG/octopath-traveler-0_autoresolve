<img src="images/icon.png" alt="Octopath Traveler 0" width="96" height="96">

# 歧路旅人 0 · 等级压制自动结算

[网页说明](https://othinusg.github.io/octopath-traveler-0_autoresolve/zh.html) · [English](README.en.md) · [使用说明](mod/README.md) · [验证记录](docs/VALIDATION.md)

基于 **UE4SS 实验版 3.0.1-1181-g83330635** 的《歧路旅人 0》Mod：队伍等级明显高于普通暗雷敌人时，跳过战斗转场，直接获得奖励。

适配目标：**Windows Steam build 22012444**（AppID **3014320**）。当前版本：**0.1.0**。

![游戏内截图：中文界面的自动结算奖励提示](images/ot0rs_zh.png)

## 实现效果

- **仅处理地图行走触发的普通随机暗雷**。Boss、剧情战、固定事件战不参与；无法确认的遭遇保留正常战斗。
- 出战队伍最高等级达到「地区危险等级与抽中敌群最高等级的较高值 + 等级差」时生效；默认等级差为 **1**。前后排出战角色参与等级判断，访客不参与。
- 使用游戏原生抽敌流程，保留稀有／首次敌群抽选。
- 发放 EXP、JP、金钱和概率掉落，计算已验证的被动、装备、村庄加成与训练奖励，按原生规则取整。掉落遵守原生结算数量与背包容量限制。
- 倒下的出战角色不获得 EXP/JP；出战与训练角色真正升级时，恢复该角色 HP/SP。
- **F6** 开关并保存状态，使用原生非阻塞提示。提示跟随界面语言：中文界面显示中文，其他语言使用英文，与语音设置无关。
- 限回合料理生效、任务专用掉落等情况保留正常战斗。

## 安装

本仓库和打包工具生成的安装包均**不包含 UE4SS 加载器**。仓库 ZIP 为源码，需先构建原生 DLL，不能直接复制源码文件夹完成安装。

1. 退出游戏，先单独安装官方 [UE4SS 实验版](https://github.com/UE4SS-RE/RE-UE4SS/releases/tag/experimental-latest)。本项目按 `UE4SS_v3.0.1-1181-g83330635.zip` 验证，不能将持续更新的实验版视为同一版本。
2. [下载仓库 ZIP](https://github.com/OthinusG/octopath-traveler-0_autoresolve/archive/refs/heads/main.zip)，解压后按[开发说明](docs/DEVELOPMENT.md)准备 Python 3、Clang、LLVM dlltool 与 lld-link，在源码目录执行：

   ```sh
   python3 tools/package.py
   ```

3. 解压生成的 `dist/OT0AutoResolve-0.1.0.zip`，将其中 `OT0AutoResolve` 文件夹复制到：

   ```text
   Octopath_Traveler0/Binaries/Win64/ue4ss/Mods/
   ```

4. 保留 Mod 内的 `enabled.txt`。若现有 UE4SS 配置使用 `ue4ss/Mods/mods.txt`，启用以下条目并保留其他 Mod：

   ```text
   OT0AutoResolve : 1
   ```

5. 启动游戏，进入地图即可生效；按 F6 可以关闭。

**升级已有安装**：退出游戏后，只替换 `OT0AutoResolve/Scripts`（含 `ot0_native.dll`），保留自己的 `config.ini`，不要覆盖其他 Mod、加载器或 UE4SS 设置。

## 设置

配置文件：`ue4ss/Mods/OT0AutoResolve/config.ini`。手动修改后重启游戏。

```ini
[General]
Enabled=1
LevelMargin=1
ShowUI=1
```

| 设置 | 含义 |
| --- | --- |
| `Enabled` | 1 开启、0 关闭；F6 会保存此值 |
| `LevelMargin` | 等级差，0–99；默认 1，0 允许同等级结算 |
| `ShowUI` | 1 显示奖励提示，0 隐藏 |

F6 为固定快捷键。Mod 不直接编辑存档；游戏原生自动存档仍按原流程运行。可执行文件或调用位置不匹配时拒绝安装钩子；奖励校验失败时关闭后续自动结算，记录在 `ue4ss/Mods/OT0AutoResolve/OT0AutoResolve.log`。

## 验证与范围

Lua 启动测试、C++ 逻辑测试、Lua 语法检查、Windows x64 编译及安装包检查通过，覆盖等级门槛、敌群保护、掉落数量、训练取整、语言选择与 F6 调度。源码包独立解压后可重建相同 DLL，无需提取游戏资源或 UE4SS SDK。

指定版本已完成多次实际暗雷结算，检查 EXP、JP、金钱、训练奖励、升级 HP/SP 恢复、F6 持久化和等级差不足时的原生战斗；最终安装文件与构建产物一致，冷启动通过。

**记录样本中尚未观察到正向物品掉落，也未逐场实测所有 Boss／剧情／固定战斗。** 保护范围依据原生调用流程和敌群检查，不能据此宣称其他游戏版本兼容。详见[验证范围](docs/VALIDATION.md)。

不模拟战斗表现奖励、偷窃、捕获或首次怪物发现等未经验证的战斗效果；限回合料理与任务专用掉落保留正常战斗。

## 源码与打包

日常测试和打包不需要游戏资源、PDB、SDK 或 UE4SS 源码。遇敌选择与奖励通过已验证的原生接口读取，无需静态敌人目录。

```sh
# Lua syntax and bootstrap
luac -p mod/OT0AutoResolve/Scripts/main.lua
lua tests/test_bootstrap.lua "$PWD"

# C++17 logic tests
clang++ -std=c++17 -Wall -Wextra -Werror tests/test_logic.cpp -o /tmp/ot0-test-logic
/tmp/ot0-test-logic

# Python 3, Clang, LLVM dlltool and lld-link
python3 tools/package.py
```

输出：`dist/OT0AutoResolve-0.1.0.zip`、`dist/OT0AutoResolve-0.1.0-source.zip` 和 `dist/SHA256SUMS.txt`。安装包包含 Mod 自身的原生 DLL，不含 UE4SS 加载器。构建依赖与架构约束见[开发说明](docs/DEVELOPMENT.md)和[技术规格](docs/Tech-Spec.md)。

```text
mod/OT0AutoResolve/   Mod bootstrap and default configuration
mod/README.md        Chinese user guide
native/              OT0 native hooks and supported-build checks
tests/               Lua bootstrap and C++ logic tests
tools/               Native build and packaging tools
docs/                Development, validation and bilingual homepage
images/              Project icon and gameplay screenshots
LICENSES/            Upstream license notice
```

## 卸载

退出游戏，删除 `ue4ss/Mods/OT0AutoResolve`，并移除自己在 `ue4ss/Mods/mods.txt` 中添加的对应条目。其他 Mod 仍需要 UE4SS 时保留加载器。

## 来源与许可证

本项目参考同系列自动结算 Mod 的交互思路，针对 OT0 单独验证原生遇敌流程、奖励接口和游戏版本，不复用 OT1 的接口签名或静态数据。仓库由 UE4SS fork 整理而来，保留上游历史和[原 MIT 许可证说明](LICENSES/UE4SS-MIT.txt)；本 Mod 源码未授予再分发许可，默认保留所有权利。参见 [NOTICE](NOTICE.md)。

仓库不包含游戏原始资源、SDK／PDB 提取文件、存档或 UE4SS 二进制。本项目是非官方 Mod，与游戏发行方无关联。
