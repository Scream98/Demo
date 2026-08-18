#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CombatAttributeSet.generated.h"

#define COMBAT_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class GAMEDEMO_API UCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCombatAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	COMBAT_ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	COMBAT_ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxHealth)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
