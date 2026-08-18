#pragma once

#include "CoreMinimal.h"
#include "CombatScriptBridge.generated.h"

class ACS;
class UScriptDrivenGameplayAbility;

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
	SingleLaunch,
	AfterLaunch,
	Combo,
	Ending
};

UCLASS(Blueprintable)
class GAMEDEMO_API UCombatScriptBridge : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	void Initialize(ACS* InCharacter);
	void Shutdown();

	bool RequestCommonAtkInput();
	bool BeginAttack(int32 AttackIndex, UScriptDrivenGameplayAbility* Ability);
	void NotifyAbilityEnded(UScriptDrivenGameplayAbility* Ability);
	bool CanMove() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanStartAttack(const FName& AttackId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Script")
	FName GetAttackId(int32 AttackIndex) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Script")
	FName GetAnimationId(int32 AttackIndex) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Script")
	int32 GetNextAttackIndex(int32 AttackIndex) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Script")
	float GetPhaseDuration(int32 AttackIndex, int32 PhaseIndex) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Script")
	void OnPhaseChanged(int32 AttackIndex, int32 PhaseIndex);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackAnimation(const FName& AttackId, const FName& AnimationId) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentAttackIndex() const { return CurrentAttackIndex; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	ECombatPhase GetCurrentPhase() const { return CurrentPhase; }

protected:
	virtual FName GetAttackId_Implementation(int32 AttackIndex) const;
	virtual FName GetAnimationId_Implementation(int32 AttackIndex) const;
	virtual int32 GetNextAttackIndex_Implementation(int32 AttackIndex) const;
	virtual float GetPhaseDuration_Implementation(int32 AttackIndex, int32 PhaseIndex) const;
	virtual void OnPhaseChanged_Implementation(int32 AttackIndex, int32 PhaseIndex);

	void SetPhase(ECombatPhase NewPhase);
	void AdvancePhase();
	void FinishCurrentAttack(bool bWasCancelled);
	void ClearCombatTags();

	UPROPERTY(Transient)
	TObjectPtr<ACS> Character;

	UPROPERTY(Transient)
	TObjectPtr<UScriptDrivenGameplayAbility> ActiveAbility;

	FTimerHandle PhaseTimer;
	int32 CurrentAttackIndex = INDEX_NONE;
	ECombatPhase CurrentPhase = ECombatPhase::SingleLaunch;
	bool bHasPhase = false;
	bool bComboInputBuffered = false;
};
