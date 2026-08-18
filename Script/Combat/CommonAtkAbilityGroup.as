// The editable CommonAtk policy. C++ owns GAS and falls back to the same defaults
// when this script class is unavailable during early startup.
class UCommonAtkAbilityGroup : UCombatScriptBridge
{
    UFUNCTION(BlueprintOverride)
    FName GetAttackId(int32 AttackIndex) const
    {
        if (AttackIndex == 0) return FName("CommonAtk_01");
        if (AttackIndex == 1) return FName("CommonAtk_02");
        return FName("CommonAtk_03");
    }

    UFUNCTION(BlueprintOverride)
    FName GetAnimationId(int32 AttackIndex) const
    {
        if (AttackIndex == 0) return FName("002000");
        if (AttackIndex == 1) return FName("002001");
        return FName("002002");
    }

    UFUNCTION(BlueprintOverride)
    int32 GetNextAttackIndex(int32 AttackIndex) const
    {
        if (AttackIndex >= 0 && AttackIndex < 2) return AttackIndex + 1;
        return -1;
    }

    UFUNCTION(BlueprintOverride)
    float GetPhaseDuration(int32 AttackIndex, int32 PhaseIndex) const
    {
        if (PhaseIndex == 0) return 0.10f;
        if (PhaseIndex == 1) return 0.12f;
        if (PhaseIndex == 2) return 0.28f;
        return 0.20f;
    }
};
