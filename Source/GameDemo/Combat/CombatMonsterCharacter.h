#pragma once

#include "CoreMinimal.h"
#include "CS.h"
#include "CombatMonsterCharacter.generated.h"

/**
 * Temporary monster host. It reuses ACS combat/GAS behavior while keeping
 * player input disabled; AI or AngelScript can request abilities explicitly.
 */
UCLASS(Blueprintable)
class GAMEDEMO_API ACombatMonsterCharacter : public ACS
{
	GENERATED_BODY()

public:
	ACombatMonsterCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
