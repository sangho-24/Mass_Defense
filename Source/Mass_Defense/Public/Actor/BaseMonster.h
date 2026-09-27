#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DamageableInterface.h"
#include "BaseMonster.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class ASplinePathActor;
class USplineComponent;
class UAnimSequence;

UCLASS()
class MASS_DEFENSE_API ABaseMonster : public AActor
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|SetUp")
	bool bUseVAT = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|SetUp")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|SetUp")
	float CurrentHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SetUp")
	float MoveSpeed = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|SetUp")
	TObjectPtr<UAnimSequence> DeathAnimAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|VAT")
	float WalkAnimStartFrame = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|VAT")
	float WalkAnimEndFrames = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|VAT")
	float DeathAnimStartFrame = 31.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|VAT")
	float DeathAnimEndFrames = 56.0f;

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
	// 메니저 참조용 게터
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }
	float GetMoveSpeed() const { return MoveSpeed; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetWalkAnimStartFrame() const { return WalkAnimStartFrame; }
	float GetWalkAnimEndFrames() const { return WalkAnimEndFrames; }
	float GetDeathAnimStartFrame() const { return DeathAnimStartFrame; }
	float GetDeathAnimEndFrames() const { return DeathAnimEndFrames; }
	// 인터페이스
	virtual float ApplyDamage(float InDamageAmount, AActor* InAttacker) override;
	virtual void Death(AActor* InKiller) override;
	virtual bool IsDead() const override { return bIsDead; }
};


