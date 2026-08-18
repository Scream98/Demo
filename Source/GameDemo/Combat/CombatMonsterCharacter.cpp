#include "CombatMonsterCharacter.h"

ACombatMonsterCharacter::ACombatMonsterCharacter()
{
	CharacterId = TEXT("TestMonster");
	SkeletonId = TEXT("TestMonster");
	CombatProfileId = TEXT("Temporary.Monster");
	AbilitySetId = TEXT("CommonAtk");
	FactionId = TEXT("Faction.Enemy");
	bEnablePlayerCombatInput = false;
}

void ACombatMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Monster actions come from AI/AngelScript, never from the player mapping.
}
