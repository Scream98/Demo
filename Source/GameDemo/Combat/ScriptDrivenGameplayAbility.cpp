#include "ScriptDrivenGameplayAbility.h"

#include "CS.h"

UScriptDrivenGameplayAbility::UScriptDrivenGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UScriptDrivenGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACS* Character = ActorInfo != nullptr ? Cast<ACS>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (Character == nullptr || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!Character->BeginCommonAttack(AttackIndex, this))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UScriptDrivenGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	ACS* Character = ActorInfo != nullptr ? Cast<ACS>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (Character != nullptr)
	{
		Character->NotifyCommonAttackAbilityEnded(this);
	}
}

void UScriptDrivenGameplayAbility::FinishFromCombat(const bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
	}
}
