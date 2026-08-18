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
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

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

	UE_LOG(LogTemp, Log, TEXT("[Combat] Character=%s CharacterId=%s Faction=%s Profile=%s AbilitySet=%s"),
		*GetName(), *CharacterId.ToString(), *FactionId.ToString(), *CombatProfileId.ToString(), *AbilitySetId.ToString());

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

	if (!bEnablePlayerCombatInput)
	{
		return;
	}

	PlayerInputComponent->BindAction(TEXT("CommonAtk"), IE_Pressed, this, &ACS::HandleCommonAtkInput);
	PlayerInputComponent->BindAxis(TEXT("Move Forward / Backward"), this, &ACS::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move Right / Left"), this, &ACS::MoveRight);

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

void ACS::DrawTemporaryCombatDebug(const int32 AttackIndex, const int32 PhaseIndex, const float Duration)
{
	if (!bEnableTemporaryCombatDebug || GetWorld() == nullptr)
	{
		return;
	}

	static const FColor PhaseColors[] =
	{
		FColor(235, 70, 70),
		FColor(245, 150, 55),
		FColor(70, 220, 120),
		FColor(70, 150, 245)
	};
	const FColor Color = PhaseColors[FMath::Clamp(PhaseIndex, 0, UE_ARRAY_COUNT(PhaseColors) - 1)];
	const FVector Origin = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector PulseCenter = Origin + Forward * TemporaryCombatDebugReach;
	const float PulseDuration = FMath::Max(0.05f, Duration);

	DrawDebugSphere(GetWorld(), PulseCenter, TemporaryCombatDebugRadius, 20, Color, false, PulseDuration, 0, 2.5f);
	DrawDebugDirectionalArrow(GetWorld(), Origin, PulseCenter, 14.0f, Color, false, PulseDuration, 0, 3.0f);

	const FString PhaseName = [&]()
	{
		switch (PhaseIndex)
		{
		case 0: return FString(TEXT("SingleLaunch"));
		case 1: return FString(TEXT("AfterLaunch"));
		case 2: return FString(TEXT("Combo"));
		case 3: return FString(TEXT("Ending"));
		default: return FString(TEXT("Unknown"));
		}
	}();
	const FString DebugText = FString::Printf(TEXT("CommonAtk_%02d | %s"), AttackIndex + 1, *PhaseName);
	DrawDebugString(GetWorld(), Origin + FVector(0.0f, 0.0f, 70.0f), DebugText, this, Color, PulseDuration, true, 1.0f);

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), PulseDuration, Color, DebugText);
	}
}

void ACS::ClearTemporaryCombatDebug()
{
	if (GEngine != nullptr)
	{
		GEngine->RemoveOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()));
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
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Combat] CommonAtk AngelScript policy loaded: %s"), *ScriptClass->GetPathName());
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

