#include "CombatAttributeSet.h"

#include "Net/UnrealNetwork.h"

UCombatAttributeSet::UCombatAttributeSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
}

void UCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Health, OldHealth);
}

void UCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxHealth, OldMaxHealth);
}
