#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DamageableInterface.h"
#include "BaseMonsterShit.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class ASplinePathActor;
class USplineComponent;
class UAnimSequence;

UCLASS()
class MASS_DEFENSE_API ABaseMonsterShit : public AActor
	, public IDamageableInterface
{
	GENERATED_BODY()
	
// 베이스 몬스터라 상속필수
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SetUp")
	TObjectPtr<ASplinePathActor> SplinePathActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	bool bUseVAT = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|SetUp")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|SetUp")
	float CurrentHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SetUp")
	float MoveSpeed = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|SetUp")
	TObjectPtr<UAnimSequence> DeathAnimAsset;
	
	bool bIsDead = false;
	float PathOffset = 0.0f;
	float CurrentSplineDistance = 0.0f;
	float CachedSplineLength = 0.0f;
	USplineComponent* CachedSplineComponent;
	

public:	
	ABaseMonsterShit();

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


