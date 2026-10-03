#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnManagerBatch.generated.h"

class ABaseMonster;
class USplineComponent;
class ASplinePathActor;
class UInstancedStaticMeshComponent;

// 몬스터 데이터 구조체
USTRUCT(BlueprintType)
struct FMonsterInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 InstanceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentDistanceAlongSpline = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PathOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MoveSpeed = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDead = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeathTime = 0.0f;
};

// LUT 구조체
USTRUCT()
struct FSplineLUTSample
{
	GENERATED_BODY()

	FTransform Transform;
	FVector RightVector;
};

// 그리드 셀 내 몬스터 인덱스 구조체
USTRUCT()
struct FSpatialGridCell
{
	GENERATED_BODY()

	TArray<int32> MonsterIndex;
};

UCLASS()
class MASS_DEFENSE_API AMonsterSpawnManagerBatch : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ISM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> ISMComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABaseMonster> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASplinePathActor> SplinePathActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float SpawnInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	int32 TotalSpawnCount = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float RandomOffset = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float MonsterYawRotationOffset = -90.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	FVector MonsterScaleOffset = FVector(1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|SetUp", meta = (AllowPrivateAccess = "true"))
	float MonsterZLocationOffset = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|LUT|SetUp", meta = (AllowPrivateAccess = "true"))
	float LUTSampleDist = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|LUT|SetUp", meta = (AllowPrivateAccess = "true"))
	bool bUseLUT = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization|SpatialGrid|SetUp", meta = (AllowPrivateAccess = "true"))
	float GridCellSize = 1000.0f;
	
	// 64비트 정수 키(CellX, CellY 결합) 기반 해시 맵
	TMap<int64, FSpatialGridCell> SpatialGrid;
	
	TArray<FSplineLUTSample> SplineLUT;
	
	// CDO에서 캐싱할 몬스터 스펙
	float CachedDefaultMoveSpeed = 400.0f;
	float CachedDefaultMaxHealth = 100.0f;
	float CachedWalkAnimStartFrame = 0.0f;
	float CachedWalkAnimEndFrames = 30.0f;
	float CachedDeathAnimStartFrame = 31.0f;
	float CachedDeathAnimEndFrames = 56.0f;
	
	TArray<FMonsterInstanceData> ActiveMonsterData;

	int32 CurrentSpawnedCount = 0;
	FTimerHandle SpawnTimerHandle;
	FTransform CachedSpawnTransform;
	
	TWeakObjectPtr<USplineComponent> CachedSplineComponent;
	float CachedSplineLength = 0.0f;
	
public:	
	AMonsterSpawnManagerBatch();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	void InitializeFromCDO();
	void InitializeSplineLUT();
	void GetLUTTransform(float InDistance, float InOffset, FTransform& OutTransform) const;
	void SpawnMonsterISM();
	void UpdateBatchSplineMovement(float DeltaTime);
	void HandleInstanceDeath(int32 DataIndex);
	
	void RebuildSpatialGrid();
	
	FORCEINLINE int64 MakeCellKey(int32 CellX, int32 CellY)
	{
		return (static_cast<int64>(CellX) << 32) | (static_cast<int64>(CellY) & 0xFFFFFFFF);
	}
	
	FORCEINLINE void GetCellCoords(const FVector& Location, int32& OutCellX, int32& OutCellY) const
	{ 
		OutCellX = FMath::FloorToInt(Location.X / GridCellSize);
		OutCellY = FMath::FloorToInt(Location.Y / GridCellSize);
	}
	
public:
	// 포탑 타겟팅
	bool FindTargetMonster(const FVector& SearchOrigin, float SearchRadius, FVector& OutTargetLocation, int32& OutTargetIndex);
	bool FindTargetMonsterSpatialGrid(const FVector& SearchOrigin, float SearchRadius, FVector& OutTargetLocation, int32& OutTargetIndex);
	// 피격 처리
	float ApplyDamageToInstance(int32 TargetIndex, float InDamageAmount, AActor* InAttacker);
	FVector GetMonsterLocation(int32 MonsterIndex) const;
	bool IsMonsterAlive(int32 MonsterIndex) const;
};
