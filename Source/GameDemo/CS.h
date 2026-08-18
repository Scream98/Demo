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

	UCombatAbilitySystemComponent* GetCombatAbilitySystemComponent() const { return CombatAbilitySystemComponent; }
	UCombatScriptBridge* GetCombatScriptBridge() const { return CombatScriptBridge; }

	bool BeginCommonAttack(int32 AttackIndex, UScriptDrivenGameplayAbility* Ability);
	void NotifyCommonAttackAbilityEnded(UScriptDrivenGameplayAbility* Ability);

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

};
