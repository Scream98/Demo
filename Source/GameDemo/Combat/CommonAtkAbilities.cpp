#include "CommonAtkAbilities.h"

namespace
{
	void AddCommonAttackTags(UGameplayAbility& Ability, const TCHAR* SpecificTag)
	{
		Ability.AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.CommonAtk")));
		Ability.AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(SpecificTag));
	}
}

UCommonAtkAbility01::UCommonAtkAbility01()
{
	AttackIndex = 0;
	AddCommonAttackTags(*this, TEXT("Ability.CommonAtk.01"));
}

UCommonAtkAbility02::UCommonAtkAbility02()
{
	AttackIndex = 1;
	AddCommonAttackTags(*this, TEXT("Ability.CommonAtk.02"));
}

UCommonAtkAbility03::UCommonAtkAbility03()
{
	AttackIndex = 2;
	AddCommonAttackTags(*this, TEXT("Ability.CommonAtk.03"));
}
