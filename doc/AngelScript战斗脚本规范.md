# AngelScript 战斗脚本规范

## 目录

```text
Demo/Script/Combat
Demo/Script/Combat/Ability
Demo/Script/Combat/Combo
Demo/Script/Combat/Damage
Demo/Script/Combat/Reaction
```

## 命名

```text
ACombatCharacter
UCombatRuntimeComponent
UCommonAtkAbilityGroup
UComboController
FCombatAttackStep
FCombatDamageRequest
FCombatHitResult
```

## 规则

- 战斗状态只能通过 Runtime Component 修改。
- 外部输入只能调用公开请求接口。
- 脚本不直接写入 GAS 属性。
- 脚本不直接执行底层物理查询。
- 伤害公式由脚本决定，最终扣血由 GAS 执行。
- 每个攻击动作必须有稳定的 `AttackId`。
- 不在多个脚本中重复保存当前 Combo 索引。

## 公共事件

```text
OnAttackStarted
OnAttackActive
OnAttackHit
OnComboQueued
OnAttackFinished
OnAttackInterrupted
OnDamageReceived
OnDeath
```

## 热重载要求

- 脚本热重载后清理旧对象引用。
- 不保存不可恢复的临时 Actor 指针。
- 当前攻击热重载时安全回到 Idle。
- 编译失败时保留上一份可运行模块，不进入半初始化状态。
