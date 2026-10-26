# 歧路旅人 0 · 等级压制自动结算

[English](../README.en.md) · [首页](../README.md) · [验证记录](../docs/VALIDATION.md)

适用于 Windows Steam 版的自动结算 Mod：当队伍等级明显高于普通地图暗雷时，跳过战斗并通过游戏原生接口发放奖励。



| 简体中文 | English |
| --- | --- |
| 中文实机截图见[首页](../README.md)。 | English screenshot on the [project homepage](../README.en.md). |

适配目标：Steam AppID **3014320**、build **22012444**、官方 UE4SS 实验版 **3.0.1-1181-g83330635**。已验证范围与限制见[验证记录](../docs/VALIDATION.md)。

## 效果与功能

- 仅处理地图行走触发的普通暗雷。Boss、剧情、固定事件和无法确认的战斗保留正常战斗。
- 出战队伍最高等级须高于地区危险等级和抽中敌群等级，差值达到 `LevelMargin`（默认 1）。
- 使用原生抽敌流程，保留稀有／首次敌群抽选。
- 通过原生接口结算 EXP、JP、金钱、概率掉落和已验证的训练奖励；角色升级后恢复该角色 HP/SP。
- **F6** 开关。中文界面显示中文提示，其他界面语言显示英文，与配音语言无关。
- 限回合料理生效、任务专用掉落等情况保留正常战斗。

## 安装

本仓库和安装包均不包含 UE4SS 加载器。先单独安装[官方 UE4SS 实验版](https://github.com/UE4SS-RE/RE-UE4SS/releases/tag/experimental-latest)，已验证归档为 `UE4SS_v3.0.1-1181-g83330635.zip`。将 `OT0AutoResolve` 放入：

```text
Octopath_Traveler0/Binaries/Win64/ue4ss/Mods/
```

保留加载器配置和其他 Mod；按当前 UE4SS 配置在 `mods.txt` 启用该 Mod。默认开启，配置文件 `config.ini` 支持 `Enabled`、`LevelMargin`（0–99）与 `ShowUI`。卸载时退出游戏，只删除此 Mod 文件夹及自己添加的对应条目。

## 源码与验证

源码、测试、构建和打包命令见 [Tech-Spec.md](../docs/Tech-Spec.md)。当前适配 Windows Steam build 22012444；遇到不匹配的可执行文件时，原生模块会拒绝安装钩子。完整实机范围见[验证记录](../docs/VALIDATION.md)：普通暗雷重复结算、奖励、训练、升级恢复、F6 与等级差不足时的原生战斗均已检查；记录样本中尚未观察到正向物品掉落，也未逐场实测所有 Boss／剧情战。

## 许可证与声明

本 Mod 源码未授予再分发许可，默认保留所有权利；[UE4SS MIT 许可](../LICENSES/UE4SS-MIT.txt)仅适用于 UE4SS，不适用于本 Mod。项目为非官方社区 Mod，与 Square Enix 无关联。
