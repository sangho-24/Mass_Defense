#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTowerShit.generated.h"

class UCapsuleComponent;
class ABaseProjectile;

UCLASS()
class MASS_DEFENSE_API ABaseTowerShit : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	TSubclassOf<ABaseProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackDamage = 35.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	bool bIsHoming = false;
	
private:
	FTimerHandle AttackTimerHandle;
	TWeakObjectPtr<AActor> CurrentTarget;
	
public:	
	ABaseTowerShit();

protected:
	virtual void BeginPlay() override;
	virtual void AttackTimer();
	AActor* FindNearestTarget() const;
	void FireAtTarget(AActor* TargetActor);
	bool IsTargetValid() const;

public:	
	virtual void Tick(float DeltaTime) override;

};
