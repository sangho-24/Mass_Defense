#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

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
	
	bool bHasHit = false;
	TWeakObjectPtr<AActor> CurrentTarget;
	
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

};
