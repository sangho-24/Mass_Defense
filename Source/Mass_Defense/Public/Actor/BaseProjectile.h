#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class AMonsterSpawnManagerBatch;

UCLASS()
class MASS_DEFENSE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float Damage = 35.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ISM|SetUp")
	float HitAcceptanceRadius = 50.0f;
	
	TWeakObjectPtr<AActor> CurrentTarget;
	bool bHasHit = false;
	bool bUseISM = false;
	
	// UPROPERTY가 없으면 이 참조를 인식 불가 -> 참조가 없으면 GC가 삭제할 수 있음 -> 댕글링
	UPROPERTY()
	TObjectPtr<AMonsterSpawnManagerBatch> TargetSpawnManager;
	int32 TargetISMIndex = INDEX_NONE;
	
public:	
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	virtual void Tick(float DeltaTime) override;
	void SetDamage(float InDamage) { Damage = InDamage; }
	void SetHomingTarget(AActor* InTargetActor);
	void SetISMTarget(AMonsterSpawnManagerBatch* InManager, int32 InTargetIndex);
	void TickISMTravel(float DeltaTime);

};
