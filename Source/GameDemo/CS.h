// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "CS.generated.h"

class UAbilitySystemComponent;
class UCombatAbilitySystemComponent;
class UCombatAttributeSet;
class UCombatScriptBridge;
class UScriptDrivenGameplayAbility;

UCLASS()
class GAMEDEMO_API ACS : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACS();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool RequestCommonAtkInput();

	void HandleCommonAtkInput();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ActivateCommonAttackIndex(int32 AttackIndex);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanMove() const;

	UFUNCTION(BlueprintPure, Category = "Character|Identity")
	FName GetCharacterId() const { return CharacterId; }

	UFUNCTION(BlueprintPure, Category = "Character|Identity")
	FName GetSkeletonId() const { return SkeletonId; }

	UFUNCTION(BlueprintPure, Category = "Character|Combat")
	FName GetCombatProfileId() const { return CombatProfileId; }

	UFUNCTION(BlueprintPure, Category = "Character|Combat")
	FName GetAbilitySetId() const { return AbilitySetId; }

	UFUNCTION(BlueprintPure, Category = "Character|Identity")
	FName GetFactionId() const { return FactionId; }

	UCombatAbilitySystemComponent* GetCombatAbilitySystemComponent() const { return CombatAbilitySystemComponent; }
	UCombatScriptBridge* GetCombatScriptBridge() const { return CombatScriptBridge; }

	bool BeginCommonAttack(int32 AttackIndex, UScriptDrivenGameplayAbility* Ability);
	void NotifyCommonAttackAbilityEnded(UScriptDrivenGameplayAbility* Ability);
	void DrawTemporaryCombatDebug(int32 AttackIndex, int32 PhaseIndex, float Duration);
	void ClearTemporaryCombatDebug();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void GrantCommonAttackAbilities();
	void InitializeCombatScriptBridge();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatAbilitySystemComponent> CombatAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatAttributeSet> CombatAttributeSet;

	UPROPERTY(Transient)
	TObjectPtr<UCombatScriptBridge> CombatScriptBridge;

	// Temporary profile fields keep BP setup editable until DataAssets are introduced.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Identity")
	FName CharacterId = TEXT("C0000");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Identity")
	FName SkeletonId = TEXT("C0000");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Combat")
	FName CombatProfileId = TEXT("Temporary.CommonAtk");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Combat")
	FName AbilitySetId = TEXT("CommonAtk");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Identity")
	FName FactionId = TEXT("Faction.Player");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	bool bEnablePlayerCombatInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Temporary Debug")
	bool bEnableTemporaryCombatDebug = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Temporary Debug", meta = (ClampMin = "1.0"))
	float TemporaryCombatDebugRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Temporary Debug", meta = (ClampMin = "1.0"))
	float TemporaryCombatDebugReach = 110.0f;

};
