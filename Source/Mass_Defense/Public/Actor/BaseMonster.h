#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DamageableInterface.h"
#include "BaseMonster.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class ASplinePathActor;
class USplineComponent;

UCLASS()
class MASS_DEFENSE_API ABaseMonster : public AActor
	, public IDamageableInterface
{
	GENERATED_BODY()
	
// 베이스 몬스터라 상속필수
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SetUp")
	TObjectPtr<ASplinePathActor> SplinePathActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|SetUp")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|SetUp")
	float CurrentHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SetUp")
	float MoveSpeed = 400.0f;
	
	bool bIsDead = false;
	float PathOffset = 0.0f;
	float CurrentSplineDistance = 0.0f;
	float CachedSplineLength = 0.0f;
	USplineComponent* CachedSplineComponent;
	

public:	
	ABaseMonster();

protected:
	virtual void BeginPlay() override;
	virtual void TickMoveAlongSpline(float DeltaTime);

public:	
	virtual void Tick(float DeltaTime) override;
	void SetSplinePathActor(ASplinePathActor* InSplinePathActor);
	void SetPathOffset(float InPathOffset);
	
public:
	virtual float ApplyDamage(float InDamageAmount, AActor* InAttacker) override;
	virtual void Death(AActor* InKiller) override;
	virtual bool IsDead() const override { return bIsDead; }
};


