#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnManagerShit.generated.h"

class ABaseMonsterShit;
class USplineComponent;
class ASplinePathActor;

UCLASS()
class MASS_DEFENSE_API AMonsterSpawnManagerShit : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABaseMonsterShit> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASplinePathActor> SplinePathActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float SpawnInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	int32 TotalSpawnCount = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float RandomOffset = 150;
	
	int32 CurrentSpawnedCount = 0;
	FTimerHandle SpawnTimerHandle;
	FTransform CachedSpawnTransform;
	
public:	
	AMonsterSpawnManagerShit();

protected:
	virtual void BeginPlay() override;
	
private:
	void SpawnMonster();

};
