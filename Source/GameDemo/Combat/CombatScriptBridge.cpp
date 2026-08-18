#include "CombatScriptBridge.h"

#include "CS.h"
#include "CombatAbilitySystemComponent.h"
#include "ScriptDrivenGameplayAbility.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace CombatTags
{
	static FGameplayTag AbilityGroup() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.CommonAtk")); }
	static FGameplayTag Ability(int32 Index) { return FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Ability.CommonAtk.%02d"), Index + 1)); }
	static FGameplayTag Phase(ECombatPhase PhaseValue)
	{
		switch (PhaseValue)
		{
		case ECombatPhase::SingleLaunch: return FGameplayTag::RequestGameplayTag(TEXT("Combat.Phase.SingleLaunch"));
		case ECombatPhase::AfterLaunch: return FGameplayTag::RequestGameplayTag(TEXT("Combat.Phase.AfterLaunch"));
		case ECombatPhase::Combo: return FGameplayTag::RequestGameplayTag(TEXT("Combat.Phase.Combo"));
		case ECombatPhase::Ending: return FGameplayTag::RequestGameplayTag(TEXT("Combat.Phase.Ending"));
		default: return FGameplayTag();
		}
	}
	static FGameplayTag Attacking() { return FGameplayTag::RequestGameplayTag(TEXT("State.Attacking")); }
	static FGameplayTag CanMove() { return FGameplayTag::RequestGameplayTag(TEXT("State.CanMove")); }
	static FGameplayTag Buffered() { return FGameplayTag::RequestGameplayTag(TEXT("State.ComboInputBuffered")); }
}

void UCombatScriptBridge::BeginDestroy()
{
	Shutdown();
	Super::BeginDestroy();
}

void UCombatScriptBridge::Initialize(ACS* InCharacter)
{
	Character = InCharacter;
	CurrentAttackIndex = INDEX_NONE;
	bHasPhase = false;
	bComboInputBuffered = false;
}

void UCombatScriptBridge::Shutdown()
{
	if (Character != nullptr && Character->GetWorld() != nullptr)
	{
		Character->GetWorld()->GetTimerManager().ClearTimer(PhaseTimer);
	}

	ClearCombatTags();
	ActiveAbility = nullptr;
	Character = nullptr;
	CurrentAttackIndex = INDEX_NONE;
	bHasPhase = false;
	bComboInputBuffered = false;
}

bool UCombatScriptBridge::RequestCommonAtkInput()
{
	if (Character == nullptr)
	{
		return false;
	}

	if (ActiveAbility == nullptr)
	{
		return Character->ActivateCommonAttackIndex(0);
	}

	if (CurrentPhase != ECombatPhase::Combo)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Combat] CommonAtk input ignored outside Combo phase."));
		return false;
	}

	const int32 NextAttackIndex = GetNextAttackIndex(CurrentAttackIndex);
	if (NextAttackIndex == INDEX_NONE)
	{
		return false;
	}

	bComboInputBuffered = true;
	if (Character->GetCombatAbilitySystemComponent() != nullptr)
	{
		Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Buffered());
	}
	UE_LOG(LogTemp, Log, TEXT("[Combat] %s buffered next attack."), *GetAttackId(CurrentAttackIndex).ToString());
	return true;
}

bool UCombatScriptBridge::BeginAttack(const int32 AttackIndex, UScriptDrivenGameplayAbility* Ability)
{
	if (Character == nullptr || Ability == nullptr || ActiveAbility != nullptr || AttackIndex < 0)
	{
		return false;
	}

	CurrentAttackIndex = AttackIndex;
	ActiveAbility = Ability;
	bComboInputBuffered = false;
	if (Character->GetCombatAbilitySystemComponent() != nullptr)
	{
		Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::AbilityGroup());
		Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Ability(AttackIndex));
		Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Attacking());
	}

	UE_LOG(LogTemp, Log, TEXT("[Combat] %s started."), *GetAttackId(AttackIndex).ToString());
	SetPhase(ECombatPhase::SingleLaunch);
	return true;
}

void UCombatScriptBridge::NotifyAbilityEnded(UScriptDrivenGameplayAbility* Ability)
{
	if (Ability == ActiveAbility)
	{
		ActiveAbility = nullptr;
		ClearCombatTags();
		CurrentAttackIndex = INDEX_NONE;
		bHasPhase = false;
		bComboInputBuffered = false;
	}
}

bool UCombatScriptBridge::CanMove() const
{
	return !IsAttacking() || CurrentPhase == ECombatPhase::Ending;
}

bool UCombatScriptBridge::CanStartAttack(const FName& AttackId) const
{
	if (Character == nullptr || ActiveAbility != nullptr || AttackId.IsNone())
	{
		return false;
	}

	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (GetAttackId(Index) == AttackId)
		{
			return true;
		}
	}

	return false;
}

void UCombatScriptBridge::PlayAttackAnimation(const FName& AttackId, const FName& AnimationId) const
{
	UE_LOG(LogTemp, Log, TEXT("[Combat] %s PlayAnimation AttackId=%s AnimationId=%s"), TEXT("CommonAtk"), *AttackId.ToString(), *AnimationId.ToString());
}

bool UCombatScriptBridge::IsAttacking() const
{
	return ActiveAbility != nullptr;
}

void UCombatScriptBridge::SetPhase(const ECombatPhase NewPhase)
{
	if (Character == nullptr || ActiveAbility == nullptr)
	{
		return;
	}

	if (Character->GetCombatAbilitySystemComponent() != nullptr && bHasPhase)
	{
		Character->GetCombatAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::Phase(CurrentPhase));
	}

	CurrentPhase = NewPhase;
	bHasPhase = true;

	if (Character->GetCombatAbilitySystemComponent() != nullptr)
	{
		Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Phase(CurrentPhase));
		if (NewPhase == ECombatPhase::Ending)
		{
			Character->GetCombatAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::Attacking());
			Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::CanMove());
		}
		else
		{
			Character->GetCombatAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Attacking());
			Character->GetCombatAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::CanMove());
		}
	}

	const FName AttackId = GetAttackId(CurrentAttackIndex);
	const float Duration = FMath::Max(0.01f, GetPhaseDuration(CurrentAttackIndex, static_cast<int32>(NewPhase)));
	UE_LOG(LogTemp, Log, TEXT("[Combat] %s Phase=%s"), *AttackId.ToString(), *UEnum::GetValueAsString(NewPhase));
	if (NewPhase == ECombatPhase::SingleLaunch)
	{
		PlayAttackAnimation(AttackId, GetAnimationId(CurrentAttackIndex));
	}
	Character->DrawTemporaryCombatDebug(CurrentAttackIndex, static_cast<int32>(NewPhase), Duration);
	OnPhaseChanged(CurrentAttackIndex, static_cast<int32>(NewPhase));

	if (Character->GetWorld() != nullptr)
	{
		Character->GetWorld()->GetTimerManager().ClearTimer(PhaseTimer);
		FTimerDelegate PhaseDelegate;
		PhaseDelegate.BindLambda([this]()
		{
			AdvancePhase();
		});
		Character->GetWorld()->GetTimerManager().SetTimer(PhaseTimer, PhaseDelegate, Duration, false);
		UE_LOG(LogTemp, Verbose, TEXT("[Combat] PhaseTimer Attack=%s Duration=%.2f World=%s"),
			*AttackId.ToString(), Duration, *Character->GetWorld()->GetName());
	}
}

void UCombatScriptBridge::AdvancePhase()
{
	if (ActiveAbility == nullptr)
	{
		return;
	}

	switch (CurrentPhase)
	{
	case ECombatPhase::SingleLaunch:
		SetPhase(ECombatPhase::AfterLaunch);
		break;
	case ECombatPhase::AfterLaunch:
		SetPhase(ECombatPhase::Combo);
		break;
	case ECombatPhase::Combo:
		if (bComboInputBuffered)
		{
			const int32 NextAttackIndex = GetNextAttackIndex(CurrentAttackIndex);
			bComboInputBuffered = false;
			if (Character->GetCombatAbilitySystemComponent() != nullptr)
			{
				Character->GetCombatAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::Buffered());
			}
			FinishCurrentAttack(false);
			if (Character != nullptr)
			{
				Character->ActivateCommonAttackIndex(NextAttackIndex);
			}
		}
		else
		{
			SetPhase(ECombatPhase::Ending);
		}
		break;
	case ECombatPhase::Ending:
		FinishCurrentAttack(false);
		break;
	default:
		FinishCurrentAttack(true);
		break;
	}
}

void UCombatScriptBridge::FinishCurrentAttack(const bool bWasCancelled)
{
	if (Character != nullptr && Character->GetWorld() != nullptr)
	{
		Character->GetWorld()->GetTimerManager().ClearTimer(PhaseTimer);
		Character->ClearTemporaryCombatDebug();
	}

	UScriptDrivenGameplayAbility* Ability = ActiveAbility;
	ClearCombatTags();
	ActiveAbility = nullptr;
	CurrentAttackIndex = INDEX_NONE;
	bHasPhase = false;
	bComboInputBuffered = false;

	if (Ability != nullptr)
	{
		Ability->FinishFromCombat(bWasCancelled);
	}
}

void UCombatScriptBridge::ClearCombatTags()
{
	if (Character == nullptr || Character->GetCombatAbilitySystemComponent() == nullptr)
	{
		return;
	}

	UCombatAbilitySystemComponent* ASC = Character->GetCombatAbilitySystemComponent();
	ASC->RemoveLooseGameplayTag(CombatTags::AbilityGroup());
	ASC->RemoveLooseGameplayTag(CombatTags::Attacking());
	ASC->RemoveLooseGameplayTag(CombatTags::CanMove());
	ASC->RemoveLooseGameplayTag(CombatTags::Buffered());
	for (int32 Index = 0; Index < 3; ++Index)
	{
		ASC->RemoveLooseGameplayTag(CombatTags::Ability(Index));
	}
	for (int32 PhaseIndex = 0; PhaseIndex < 4; ++PhaseIndex)
	{
		ASC->RemoveLooseGameplayTag(CombatTags::Phase(static_cast<ECombatPhase>(PhaseIndex)));
	}
}

FName UCombatScriptBridge::GetAttackId_Implementation(const int32 AttackIndex) const
{
	return FName(*FString::Printf(TEXT("CommonAtk_%02d"), AttackIndex + 1));
}

FName UCombatScriptBridge::GetAnimationId_Implementation(const int32 AttackIndex) const
{
	return FName(*FString::Printf(TEXT("%06d"), 2000 + AttackIndex));
}

int32 UCombatScriptBridge::GetNextAttackIndex_Implementation(const int32 AttackIndex) const
{
	return AttackIndex >= 0 && AttackIndex < 2 ? AttackIndex + 1 : INDEX_NONE;
}

float UCombatScriptBridge::GetPhaseDuration_Implementation(const int32 AttackIndex, const int32 PhaseIndex) const
{
	static const float Durations[] = { 0.10f, 0.12f, 0.28f, 0.20f };
	return PhaseIndex >= 0 && PhaseIndex < UE_ARRAY_COUNT(Durations) ? Durations[PhaseIndex] : 0.10f;
}

void UCombatScriptBridge::OnPhaseChanged_Implementation(const int32 AttackIndex, const int32 PhaseIndex)
{
}
