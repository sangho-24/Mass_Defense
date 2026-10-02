#include "Actor/MonsterSpawnManagerBatch.h"
#include "Actor/BaseMonster.h"
#include "Actor/SplinePathActor.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
// #include "Kismet/GameplayStatics.h"

AMonsterSpawnManagerBatch::AMonsterSpawnManagerBatch()
{
	PrimaryActorTick.bCanEverTick = true;

	ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMComponent"));
	ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ISMComponent->SetGenerateOverlapEvents(false);
	RootComponent = ISMComponent;

	// 인스턴스 커스텀 데이터 슬롯 4개 (0: 루프, 1: 시간 오프셋, 2: 시작 프레임, 3: 끝 프레임)
	ISMComponent->NumCustomDataFloats = 4;
}

void AMonsterSpawnManagerBatch::BeginPlay()
{
	Super::BeginPlay();

	if (!SplinePathActor || !SplinePathActor->GetSplineComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("경로 스플라인 액터가 없슴다"));
		return;
	}
	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("몬스터 클래스가 없슴다"));
		return;
	}

	ISMComponent->SetRemoveSwap();
	UE_LOG(LogTemp,Warning,TEXT("ISM SupportsRemoveSwap = %s"),ISMComponent->SupportsRemoveSwap() ? TEXT("TRUE") : TEXT("FALSE"));
	
	CachedSplineComponent = SplinePathActor->GetSplineComponent();
	CachedSplineLength = CachedSplineComponent->GetSplineLength();
	CachedSpawnTransform = CachedSplineComponent->GetTransformAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	CachedSpawnTransform.AddToTranslation(FVector(0.0f, 0.0f, MonsterZLocationOffset));
	CachedSpawnTransform.SetScale3D(MonsterScaleOffset);
	CachedSpawnTransform.ConcatenateRotation(FRotator(0.0f, MonsterYawRotationOffset, 0.0f).Quaternion());

	// CDO로부터 스태틱 메시 및 초기 스텟 추출
	InitializeFromCDO();
	// LUT 계산
	InitializeSplineLUT();
	// 스폰 주기 타이머 가동
	GetWorld()->GetTimerManager().SetTimer(
	SpawnTimerHandle,
	this,
	&AMonsterSpawnManagerBatch::SpawnMonsterISM,
	SpawnInterval,
	true
	);
}

void AMonsterSpawnManagerBatch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CachedSplineComponent.IsValid() && ActiveMonsterData.Num() > 0)
	{
		UpdateBatchSplineMovement(DeltaTime);
	}
}

void AMonsterSpawnManagerBatch::InitializeFromCDO()
{
	// 월드 스폰 없이 원본 클래스 기본 객체(CDO) 획득
	const ABaseMonster* DefaultMonster = MonsterClass->GetDefaultObject<ABaseMonster>();
	if (!DefaultMonster)
	{
		return;
	}

	// 1. 원본 클래스의 StaticMeshComponent로부터 VAT 스태틱 메시 자동 장착
	if (UStaticMeshComponent* TemplateMeshComponent = DefaultMonster->GetStaticMeshComponent())
	{
		if (UStaticMesh* MeshAsset = TemplateMeshComponent->GetStaticMesh())
		{
			ISMComponent->SetStaticMesh(MeshAsset);
			UE_LOG(LogTemp, Log, TEXT("[SpawnManager] CDO로부터 스태틱 메시(%s) 바인딩 성공!"), *MeshAsset->GetName());
		}
	}

	// 2. 기본 스펙 및 애니메이션 프레임 데이터 캐싱
	CachedDefaultMoveSpeed = DefaultMonster->GetMoveSpeed();
	CachedDefaultMaxHealth = DefaultMonster->GetMaxHealth();
	CachedWalkAnimStartFrame = DefaultMonster->GetWalkAnimStartFrame();
	CachedWalkAnimEndFrames = DefaultMonster->GetWalkAnimEndFrames();
	CachedDeathAnimStartFrame = DefaultMonster->GetDeathAnimStartFrame();
	CachedDeathAnimEndFrames = DefaultMonster->GetDeathAnimEndFrames();
}

void AMonsterSpawnManagerBatch::InitializeSplineLUT()
{
	if (!CachedSplineComponent.IsValid() || CachedSplineLength <= 0.0f || LUTSampleDist <= 0.0f)
	{
		return;
	}
	// SampleDist단위로 나눈 샘플 개수. 0도 포함되니 +1 해줘야 함.
	const int32 SampleCount = FMath::CeilToInt(CachedSplineLength / LUTSampleDist) + 1;
	// 실제 원소는 0개, SampleCount만큼의 공간만 확보.
	SplineLUT.Empty(SampleCount);

	const FQuat YawRotation = FRotator(0.0f, MonsterYawRotationOffset, 0.0f).Quaternion();

	for (int32 i = 0; i < SampleCount; ++i)
	{
		const float CurrentDistance = FMath::Min(static_cast<float>(i) * LUTSampleDist, CachedSplineLength);
		FTransform SplineTransform = CachedSplineComponent->GetTransformAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);

		FSplineLUTSample LUTSample;
		// 오프셋을 위한 라이트 백터와 스태틱 메시 회전까지 적용
		LUTSample.RightVector = SplineTransform.GetRotation().GetRightVector();
		SplineTransform.SetRotation(SplineTransform.GetRotation() * YawRotation);
		LUTSample.Transform = SplineTransform;
		
		SplineLUT.Add(LUTSample);
	}
}

void AMonsterSpawnManagerBatch::GetLUTTransform(float InDistance, float InOffset, FTransform& OutTransform) const
{
	if (SplineLUT.Num() == 0)
	{
		return;
	}
	// 배열 범위 초과 방지
	const float ClampedDistance = FMath::Clamp(InDistance, 0.0f, CachedSplineLength);
	const float NormalizedDistance = ClampedDistance / LUTSampleDist;

	const int32 IndexA = FMath::FloorToInt(NormalizedDistance);
	// 딱 맞아떨어질 때, Index+1하면 배열범위초과. 그걸 방지하기 위한 Min.
	const int32 IndexB = FMath::Min(IndexA + 1, SplineLUT.Num() - 1);
	const float Alpha = NormalizedDistance - static_cast<float>(IndexA);

	const FSplineLUTSample& SampleA = SplineLUT[IndexA];
	const FSplineLUTSample& SampleB = SplineLUT[IndexB];

	// 트랜스폼 선형 보간 (트렌스폼 구조체안 회전은 Lerp 안됨)
	OutTransform = SampleA.Transform;
	OutTransform.BlendWith(SampleB.Transform, Alpha);

	// 횡방향 오프셋 벡터 보간 적용
	const FVector BlendedRight = FMath::Lerp(SampleA.RightVector, SampleB.RightVector, Alpha).GetSafeNormal();
	OutTransform.AddToTranslation(BlendedRight * InOffset);
	OutTransform.AddToTranslation(FVector(0.0f, 0.0f, MonsterZLocationOffset));
}

void AMonsterSpawnManagerBatch::SpawnMonsterISM()
{
	if (CurrentSpawnedCount >= TotalSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("총 %d 마리 스폰 완료!"), CurrentSpawnedCount);
		return;
	}

	if (!ISMComponent || !CachedSplineComponent.IsValid() || SplineLUT.Num() == 0)
	{
		return;
	}

	const float Offset = FMath::RandRange(-RandomOffset, RandomOffset);
	FTransform InitialTransform;
	if (bUseLUT)
	{
		GetLUTTransform(0.0f, Offset, InitialTransform);
	}
	else
	{
		InitialTransform = CachedSpawnTransform;
		InitialTransform.AddToTranslation(CachedSpawnTransform.GetRotation().GetRightVector() * Offset);
	}

	// ISM 인스턴스 등록
	const int32 NewInstanceIndex = ISMComponent->AddInstance(InitialTransform, true);

	// VAT CPD 초기화: 걷기 애니메이션 설정 (CDO에서 캐싱한 프레임 사용)
	ISMComponent->SetCustomDataValue(NewInstanceIndex, 0, 1.0f, false);
	ISMComponent->SetCustomDataValue(NewInstanceIndex, 1, FMath::FRandRange(0.0f, 1.0f), false);
	ISMComponent->SetCustomDataValue(NewInstanceIndex, 2, CachedWalkAnimStartFrame, false);
	ISMComponent->SetCustomDataValue(NewInstanceIndex, 3, CachedWalkAnimEndFrames, true);

	// 데이터 배열에 등록
	FMonsterInstanceData NewData;
	NewData.InstanceIndex = NewInstanceIndex;
	NewData.CurrentDistanceAlongSpline = 0.0f;
	NewData.PathOffset = Offset;
	NewData.MoveSpeed = CachedDefaultMoveSpeed;
	NewData.MaxHealth = CachedDefaultMaxHealth;
	NewData.CurrentHealth = CachedDefaultMaxHealth;
	NewData.bIsDead = false;

	ActiveMonsterData.Add(NewData);
	CurrentSpawnedCount++;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			5.0f,
			FColor::Yellow,
			FString::Printf(TEXT("현재 스폰: %d"), CurrentSpawnedCount)
		);
	}
}

void AMonsterSpawnManagerBatch::UpdateBatchSplineMovement(float DeltaTime)
{
	const float CurrentWorldTime = GetWorld()->GetTimeSeconds();
	
	// 반복문 역순 순회
	// 중간에 사망한 인스턴스 제거하여 앞으로 당겨지더라도, 순회에서 스킵되거나 배열 범위 초과되는 일 없도록 하기 위함.
	for (int32 i = ActiveMonsterData.Num() - 1; i >= 0; --i)
	{
		FMonsterInstanceData& Data = ActiveMonsterData[i];

		if (Data.bIsDead)
		{
			// 사망 후 3초 경과 시 인스턴스 제거
			if (CurrentWorldTime - Data.DeathTime >= 3.0f)
			{
				const int32 RemovedIndex = Data.InstanceIndex;
				ISMComponent->RemoveInstance(Data.InstanceIndex);
				ActiveMonsterData.RemoveAtSwap(i);

				// Swap 보정, 마지막 인덱스가 제거되었다면 swap이 없으므로 보정할 필요도 없음
				if (i < ActiveMonsterData.Num())
				{
					ActiveMonsterData[i].InstanceIndex = RemovedIndex;
				}
				continue;
			}
		}
		else
		{
			// 살아있는 인스턴스 전진
			Data.CurrentDistanceAlongSpline += Data.MoveSpeed * DeltaTime;
			if (Data.CurrentDistanceAlongSpline >= CachedSplineLength)
			{
				Data.CurrentDistanceAlongSpline = CachedSplineLength;
			}
		}

		FTransform SplineTransform;
		if (bUseLUT)
		{
			GetLUTTransform(Data.CurrentDistanceAlongSpline, Data.PathOffset, SplineTransform);
		}
		else
		{
			SplineTransform = CachedSplineComponent->GetTransformAtDistanceAlongSpline(Data.CurrentDistanceAlongSpline, ESplineCoordinateSpace::World);
			const FVector Right = SplineTransform.GetRotation().GetRightVector();
			SplineTransform.AddToTranslation(Right * Data.PathOffset);
			SplineTransform.AddToTranslation(FVector(0.0f, 0.0f, MonsterZLocationOffset));
			SplineTransform.ConcatenateRotation(FRotator(0.0f, MonsterYawRotationOffset, 0.0f).Quaternion());
		}
		// 스케일은 LUT에서 적용할 필요 없음. (비용 같음)
		SplineTransform.SetScale3D(MonsterScaleOffset);
		// 트랜스폼 메모리 갱신 (아직 렌더링 X)
		ISMComponent->UpdateInstanceTransform(Data.InstanceIndex, SplineTransform, true, false, false);
	}
	// 일괄 GPU 플러시 (한번에 렌더링)
	ISMComponent->MarkRenderStateDirty();
}

void AMonsterSpawnManagerBatch::HandleInstanceDeath(int32 DataIndex)
{
	if (!ActiveMonsterData.IsValidIndex(DataIndex))
	{
		return;
	}

	FMonsterInstanceData& Data = ActiveMonsterData[DataIndex];
	Data.DeathTime = GetWorld()->GetTimeSeconds();
	Data.bIsDead = true;

	// 사망 VAT 애니메이션 CPD 주입 (CDO에서 가져온 사망 프레임 사용)
	ISMComponent->SetCustomDataValue(Data.InstanceIndex, 0, 0.0f, false);
	ISMComponent->SetCustomDataValue(Data.InstanceIndex, 1, -Data.DeathTime, false);
	ISMComponent->SetCustomDataValue(Data.InstanceIndex, 2, CachedDeathAnimStartFrame, false);
	ISMComponent->SetCustomDataValue(Data.InstanceIndex, 3, CachedDeathAnimEndFrames, true);
}

bool AMonsterSpawnManagerBatch::FindTargetMonster(const FVector& SearchOrigin, float SearchRadius, FVector& OutTargetLocation, int32& OutTargetIndex)
{
	float ClosestDistSq = FMath::Square(SearchRadius);
	int32 FoundDataIndex = INDEX_NONE;
	FTransform FoundTransform;

	for (int32 i = 0; i < ActiveMonsterData.Num(); ++i)
	{
		const FMonsterInstanceData& Data = ActiveMonsterData[i];
		if (Data.bIsDead)
		{
			continue;
		}

		FTransform InstanceTransform;
		ISMComponent->GetInstanceTransform(Data.InstanceIndex, InstanceTransform, true);
		const float DistSq = FVector::DistSquared(SearchOrigin, InstanceTransform.GetLocation());

		if (DistSq <= ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			FoundDataIndex = i;
			FoundTransform = InstanceTransform;
		}
	}

	if (FoundDataIndex != INDEX_NONE)
	{
		OutTargetLocation = FoundTransform.GetLocation();
		OutTargetIndex = FoundDataIndex;
		return true;
	}

	return false;
}

float AMonsterSpawnManagerBatch::ApplyDamageToInstance(int32 TargetIndex, float InDamageAmount, AActor* InAttacker)
{
	if (!ActiveMonsterData.IsValidIndex(TargetIndex))
	{
		return 0.0f;
	}

	FMonsterInstanceData& Data = ActiveMonsterData[TargetIndex];
	if (Data.bIsDead || InDamageAmount <= 0.0f)
	{
		return 0.0f;
	}
	
	const float AppliedDamage = FMath::Min(Data.CurrentHealth, InDamageAmount);
	Data.CurrentHealth -= AppliedDamage;

	if (Data.CurrentHealth <= 0.0f)
	{
		Data.CurrentHealth = 0.0f;
		HandleInstanceDeath(TargetIndex);
	}

	return AppliedDamage;
}

FVector AMonsterSpawnManagerBatch::GetMonsterLocation(int32 MonsterIndex) const
{
	if (!ActiveMonsterData.IsValidIndex(MonsterIndex) || ActiveMonsterData[MonsterIndex].bIsDead)
	{
		return FVector::ZeroVector;
	}
	if (!ISMComponent)
	{
		return FVector::ZeroVector;
	}
	FTransform OutTransform;
	// 세 번째 인자 true: 월드 스페이스 좌표로 트랜스폼 획득
	if (ISMComponent->GetInstanceTransform(ActiveMonsterData[MonsterIndex].InstanceIndex, OutTransform, true))
	{
		return OutTransform.GetLocation() + FVector(0.0f, 0.0f, -MonsterZLocationOffset);
	}
	return FVector::ZeroVector;
}

bool AMonsterSpawnManagerBatch::IsMonsterAlive(int32 MonsterIndex) const
{
	return ActiveMonsterData.IsValidIndex(MonsterIndex) && !ActiveMonsterData[MonsterIndex].bIsDead;
}
