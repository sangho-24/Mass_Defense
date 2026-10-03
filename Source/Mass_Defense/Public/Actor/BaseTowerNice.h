#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTowerNice.generated.h"


class UCapsuleComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class AMonsterSpawnManagerBatch;

USTRUCT()
struct FVirtualProjectileData
{
	GENERATED_BODY()

	FVector CurrentLocation = FVector::ZeroVector;
	FVector LastKnownTargetLocation = FVector::ZeroVector;
	int32 TargetIndex = INDEX_NONE;
	float Speed = 1500.0f;
	float Damage = 25.0f;
	
	bool bLostTarget = false;
	TWeakObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;
};

UCLASS()
class MASS_DEFENSE_API ABaseTowerNice : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackDamage = 35.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float ProjectileSpeed = 2000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float HitAcceptanceRadius = 50.0f;
	
	float HitAcceptanceRadiusSq = 2500.0f;
	
	// 나중에 포탑 대가리 생기면 쓸거
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	// float HeadRotationInterpSpeed = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX|SetUp")
	TObjectPtr<UNiagaraSystem> ProjectileNiagaraSystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX|SetUp")
	TObjectPtr<UNiagaraSystem> MuzzleFlashNiagaraSystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX|SetUp")
	TObjectPtr<UNiagaraSystem> HitNiagaraSystem;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Combat|SetUp")
	TObjectPtr<AMonsterSpawnManagerBatch> TargetSpawnManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|SetUp")
	bool bUseSpatialPartitioning = false;
	
	
private:
	FTimerHandle AttackTimerHandle;
	int32 CurrentTargetIndex = INDEX_NONE;
	FVector CurrentTargetLocation = FVector::ZeroVector;
	// bool bHasValidTarget = false;  필없지않나
	
	TArray<FVirtualProjectileData> ActiveVirtualProjectiles;
	
public:	
	ABaseTowerNice();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:	
	void AttackTimer();
	void UpdateTargetSearch();
	bool IsTargetValid();
	void FireVirtualProjectile(const FVector& TargetLocation, int32 TargetIndex);
	void TickVirtualProjectiles(float DeltaTime);
	
	void PlayMuzzleFlashEffect(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	void PlayHitEffect(const FVector& HitLocation);

};
