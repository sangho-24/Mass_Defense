#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTowerShit.generated.h"

class UCapsuleComponent;

UCLASS()
class MASS_DEFENSE_API ABaseTowerShit : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|SetUp")
	float AttackDamage = 35.0f;
	
private:
	FTimerHandle AttackTimerHandle;
	
public:	
	ABaseTowerShit();

protected:
	virtual void BeginPlay() override;
	virtual void AttackTimer();
	AActor* FindNearestTarget();
	void FireAtTarget(AActor* TargetActor);

public:	
	virtual void Tick(float DeltaTime) override;

};
