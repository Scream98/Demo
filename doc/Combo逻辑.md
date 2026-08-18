# Combo 逻辑模块

## 默认 Combo

```text
CommonAtk_01 -> CommonAtk_02 -> CommonAtk_03
```

## 状态

```text
Idle
Startup
Active
Recovery
Finished
Interrupted
```

## 脚本接口

```text
StartCombo()
TryQueueNextAttack()
HasBufferedInput()
AdvanceCombo()
ResetCombo()
InterruptCombo(Reason)
GetComboIndex()
GetCurrentAttackId()
```

## 输入缓冲

- 只保留一个下一段输入。
- 只有在 `ComboWindowOpen` 和 `ComboWindowClose` 之间接受输入。
- 窗口外输入直接丢弃。
- 第三段攻击不接受下一段输入。
- 受击、死亡、翻滚和硬直时清空缓存。
- Recovery 完成后，有缓存才推进下一段。

## 流程

```text
Idle + AttackInput
 -> StartCombo(0)
 -> Startup
 -> Active
 -> ComboWindow
 -> Recovery
 -> AdvanceCombo 或 ResetCombo
```

## 验收

- 单次按键只播放第一段。
- 窗口内连续按键可以完成三段。
- 窗口外按键不会跳段。
- 快速多次输入不会跳过攻击段。
- 第三段后回到 Idle。
- 攻击中断后重新按键从第一段开始。
