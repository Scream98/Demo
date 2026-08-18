// Fill out your copyright notice in the Description page of Project Settings.


#include "CS.h"

#include "Combat/CombatAbilitySystemComponent.h"
#include "Combat/CombatAttributeSet.h"
#include "Combat/CombatScriptBridge.h"
#include "Combat/CommonAtkAbilities.h"
#include "Combat/ScriptDrivenGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Controller.h"
#include "Math/RotationMatrix.h"
#include "UObject/UObjectGlobals.h"

// Sets default values
ACS::ACS()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatAbilitySystemComponent = CreateDefaultSubobject<UCombatAbilitySystemComponent>(TEXT("CombatAbilitySystemComponent"));
	CombatAbilitySystemComponent->SetIsReplicated(true);
	CombatAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));

}

UAbilitySystemComponent* ACS::GetAbilitySystemComponent() const
{
	return CombatAbilitySystemComponent;
}

// Called when the game starts or when spawned
void ACS::BeginPlay()
{
	Super::BeginPlay();

	CombatAbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeCombatScriptBridge();
	GrantCommonAttackAbilities();
}

// Called every frame
void ACS::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACS::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("CommonAtk"), IE_Pressed, this, &ACS::HandleCommonAtkInput);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACS::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move Right / Left"), this, &ACS::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACS::MoveRight);

}

bool ACS::RequestCommonAtkInput()
{
	return CombatScriptBridge != nullptr && CombatScriptBridge->RequestCommonAtkInput();
}

void ACS::HandleCommonAtkInput()
{
	RequestCommonAtkInput();
}

bool ACS::ActivateCommonAttackIndex(const int32 AttackIndex)
{
	if (!CombatAbilitySystemComponent)
	{
		return false;
	}

	TSubclassOf<UGameplayAbility> AbilityClass;
	switch (AttackIndex)
	{
	case 0:
		AbilityClass = UCommonAtkAbility01::StaticClass();
		break;
	case 1:
		AbilityClass = UCommonAtkAbility02::StaticClass();
		break;
	case 2:
		AbilityClass = UCommonAtkAbility03::StaticClass();
		break;
	default:
		return false;
	}

	return CombatAbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, true);
}

bool ACS::CanMove() const
{
	return CombatScriptBridge == nullptr || CombatScriptBridge->CanMove();
}

bool ACS::BeginCommonAttack(const int32 AttackIndex, UScriptDrivenGameplayAbility* Ability)
{
	return CombatScriptBridge != nullptr && CombatScriptBridge->BeginAttack(AttackIndex, Ability);
}

void ACS::NotifyCommonAttackAbilityEnded(UScriptDrivenGameplayAbility* Ability)
{
	if (CombatScriptBridge != nullptr)
	{
		CombatScriptBridge->NotifyAbilityEnded(Ability);
	}
}

void ACS::GrantCommonAttackAbilities()
{
	if (!HasAuthority() || !CombatAbilitySystemComponent)
	{
		return;
	}

	CombatAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UCommonAtkAbility01::StaticClass(), 1, INDEX_NONE, this));
	CombatAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UCommonAtkAbility02::StaticClass(), 1, INDEX_NONE, this));
	CombatAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UCommonAtkAbility03::StaticClass(), 1, INDEX_NONE, this));
}

void ACS::InitializeCombatScriptBridge()
{
	UClass* ScriptClass = FindFirstObject<UClass>(TEXT("UCommonAtkAbilityGroup"));
	if (ScriptClass == nullptr)
	{
		ScriptClass = FindFirstObject<UClass>(TEXT("CommonAtkAbilityGroup"));
	}

	if (ScriptClass == nullptr || !ScriptClass->IsChildOf(UCombatScriptBridge::StaticClass()))
	{
		ScriptClass = UCombatScriptBridge::StaticClass();
		UE_LOG(LogTemp, Warning, TEXT("[Combat] CommonAtk AngelScript class was not found; using C++ defaults."));
	}

	CombatScriptBridge = NewObject<UCombatScriptBridge>(this, ScriptClass);
	CombatScriptBridge->Initialize(this);
}

void ACS::MoveForward(const float Value)
{
	if (!CanMove() || FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller != nullptr ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ACS::MoveRight(const float Value)
{
	if (!CanMove() || FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller != nullptr ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);

}

