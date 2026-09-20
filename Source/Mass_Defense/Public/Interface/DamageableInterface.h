#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

class MASS_DEFENSE_API IDamageableInterface
{
	GENERATED_BODY()

public:
	virtual float ApplyDamage(float InDamageAmount, AActor* InAttacker) { return 0.0f; }
	virtual void SpawnFloatingDamage(const float InAmount, const bool bIsHeal, const bool bIsCritical) {}
	virtual void Death(AActor* InKiller) {}
	virtual bool IsDead() const { return false; }
	
	virtual void ApplyHitReaction(AActor* InAttacker, const FHitResult& InHitResult) {}
};
