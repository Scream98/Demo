# CommonAtk AbilityGroup

## 目标

将普通攻击作为一个可由 AngelScript 编辑的能力组。C++/GAS 提供能力激活、标签、动画和伤害执行接口。

## 攻击步骤

```text
AttackId
EldenAnimationId
AnimMontage
MontageSection
StartupTime
ActiveTime
RecoveryTime
ComboWindowStart
ComboWindowEnd
BaseDamage
AttackPowerScale
DefenseIgnore
HitRange
HitRadius
```

## 默认动作

```text
CommonAtk_01
CommonAtk_02
CommonAtk_03
```

动作编号独立配置，例如：

```text
CommonAtk_01 -> 002000
```

在 c0000 动作资源尚未完成类型和攻击帧确认前，使用：

```text
MM_Attack_01
MM_Attack_02
MM_Attack_03
```

## 脚本接口

```text
CanActivate()
Activate()
QueueInput()
SelectNextAttack()
PlayCurrentAttack()
OnAttackActive()
OnAttackHit(HitResult)
OnAttackRecovery()
Finish()
Cancel(Reason)
```

## C++/GAS 接口

```text
TryActivateAbility(AbilityTag)
AddCombatTag(State.Attacking)
PlayAttackMontage(Montage, Section)
QueryAttackHits(HitQuery)
ApplyDamage(DamageRequest)
RemoveCombatTag(State.Attacking)
```

## 结束条件

- 最后一段 Recovery 完成。
- Combo 窗口关闭且没有缓存输入。
- 角色进入死亡、硬直或受击状态。
- 动画播放失败。
- 外部系统显式取消。

## 验收

- AbilityGroup 可以启动第一段攻击。
- 脚本修改动作编号后无需编译 C++。
- 可以切换 Mannequin 和 c0000 动作。
- 每段攻击只触发一次 Active 和 Hit。
