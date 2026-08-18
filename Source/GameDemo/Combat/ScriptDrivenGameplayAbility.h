#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ScriptDrivenGameplayAbility.generated.h"

UCLASS(Abstract, Blueprintable)
class GAMEDEMO_API UScriptDrivenGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UScriptDrivenGameplayAbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 AttackIndex = INDEX_NONE;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void FinishFromCombat(bool bWasCancelled);
};
