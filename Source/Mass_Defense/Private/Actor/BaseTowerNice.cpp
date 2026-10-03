#include "Actor/BaseTowerNice.h"
#include "Actor/MonsterSpawnManagerBatch.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"



ABaseTowerNice::ABaseTowerNice()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComponent;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetupAttachment(RootComponent);
}

void ABaseTowerNice::BeginPlay()
{
	Super::BeginPlay();
	HitAcceptanceRadiusSq = FMath::Square(HitAcceptanceRadius);
	if (!TargetSpawnManager)
	{
		TargetSpawnManager = Cast<AMonsterSpawnManagerBatch>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AMonsterSpawnManagerBatch::StaticClass()));
	}
	const float RandomFirstDelay = FMath::FRandRange(0.0f, AttackInterval);
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle,this, 
		&ABaseTowerNice::AttackTimer,AttackInterval,true, RandomFirstDelay);
}

void ABaseTowerNice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TickVirtualProjectiles(DeltaTime);
}

void ABaseTowerNice::AttackTimer()
{
	if (!IsTargetValid())
	{
		UpdateTargetSearch();
	}
	
	if (CurrentTargetIndex != INDEX_NONE && TargetSpawnManager)
	{
		FireVirtualProjectile(CurrentTargetLocation, CurrentTargetIndex);
	}
}

void ABaseTowerNice::UpdateTargetSearch()
{
	// 탐색 실패 시 이전 인덱스로 인한 버그를 방지하기 위해 먼저 초기화
	CurrentTargetIndex = INDEX_NONE;
	if (!TargetSpawnManager)
	{
		return;
	}
	const FVector MyLocation = GetActorLocation();
	FVector FoundLocation = FVector::ZeroVector;
	int32 FoundIndex = INDEX_NONE;

	// 여기서 공간분할 제어
	if (bUseSpatialPartitioning)
	{
		UE_LOG(LogTemp, Warning, TEXT("공간분할 기반 타겟 탐색은 아직 없지롱"));
	}
	else
	{
		if (TargetSpawnManager->FindTargetMonster(MyLocation, AttackRange, FoundLocation, FoundIndex))
		{
			CurrentTargetIndex = FoundIndex;
			CurrentTargetLocation = FoundLocation;
		}
	}
}

bool ABaseTowerNice::IsTargetValid()
{
	if (!TargetSpawnManager || CurrentTargetIndex == INDEX_NONE)
	{
		return false;
	}
	if (!TargetSpawnManager->IsMonsterAlive(CurrentTargetIndex))
	{
		return false;
	}
	
	// 타겟이 있는 경우에 범위 밖으로 나갔는지 확인.
	// 유효할경우 현재 위치 캐싱(발사될 초기 위치)
	const FVector TargetLocation = TargetSpawnManager->GetMonsterLocation(CurrentTargetIndex);
	if (TargetLocation.IsZero())
	{
		return false;
	}
	const float Distance = FVector::Dist(GetActorLocation(), TargetLocation);
	if (Distance > AttackRange)
	{
		return false;
	}
	CurrentTargetLocation = TargetLocation;
	return true;
}

void ABaseTowerNice::FireVirtualProjectile(const FVector& TargetLocation, int32 TargetIndex)
{
	// const FVector MuzzleLocation = HeadMeshComponent ? HeadMeshComponent->GetComponentLocation() + (HeadMeshComponent->GetForwardVector() * 80.0f) : GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FVector FireLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FRotator FireRotation = (TargetLocation - FireLocation).Rotation();
	PlayMuzzleFlashEffect(FireLocation, FireRotation);
	// 나이아가라 스폰
	UNiagaraComponent* SpawnedNiagara = nullptr;
	if (ProjectileNiagaraSystem)
	{
		SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ProjectileNiagaraSystem,
		FireLocation,
		(TargetLocation - FireLocation).Rotation());
	}
	// 투사체 구조체 생성 및 등록
	FVirtualProjectileData NewData;
	NewData.CurrentLocation = FireLocation;
	NewData.LastKnownTargetLocation = TargetLocation;
	NewData.TargetIndex = TargetIndex;
	NewData.Speed = ProjectileSpeed;
	NewData.Damage = AttackDamage;
	NewData.NiagaraComponent = SpawnedNiagara;

	ActiveVirtualProjectiles.Add(NewData);
}

void ABaseTowerNice::TickVirtualProjectiles(float DeltaTime)
{
	if (ActiveVirtualProjectiles.Num() == 0 || !TargetSpawnManager)
	{
		return;
	}
	// 역순 순회 (제거가 있으니까)
	for (int32 Index = ActiveVirtualProjectiles.Num() - 1; Index >= 0; --Index)
	{
		FVirtualProjectileData& Data = ActiveVirtualProjectiles[Index];
		const FVector TargetLocation = TargetSpawnManager->GetMonsterLocation(Data.TargetIndex);
		// 타겟이 이미 죽었거나 소멸된 경우
		if (!Data.bLostTarget)
		{
			if (TargetLocation.IsZero())
			{
				Data.bLostTarget = true;
			}
			else
			{
				Data.LastKnownTargetLocation = TargetLocation;
			}
		}
		const FVector ToTarget = Data.LastKnownTargetLocation - Data.CurrentLocation;
		const float DistanceSq = ToTarget.SizeSquared();
		const float MoveStep = Data.Speed * DeltaTime;

		// 오버랩 검사 || 터널링(넘 빨라서 뚫고 나가는거) 검사
		if (DistanceSq <= HitAcceptanceRadiusSq || DistanceSq <= FMath::Square(MoveStep))
		{
			if (!Data.bLostTarget)
			{
			TargetSpawnManager->ApplyDamageToInstance(Data.TargetIndex, Data.Damage, this);
			}
			// 히트시에만 이펙트 출력할거면 위로
			PlayHitEffect(Data.LastKnownTargetLocation);
			if (Data.NiagaraComponent.IsValid())
			{
				Data.NiagaraComponent->DestroyComponent();
			}
			ActiveVirtualProjectiles.RemoveAtSwap(Index);
			continue;
		}
		// 적중 안했을때만 제곱근 연산 수행
		const float Distance = FMath::Sqrt(DistanceSq);
		// 정규화
		const FVector MoveDirection = ToTarget / Distance;
		Data.CurrentLocation += MoveDirection * MoveStep;
		// 나이아가라 이펙트 위치 동기화 (투사체 실제 이동)
		if (Data.NiagaraComponent.IsValid())
		{
			Data.NiagaraComponent->SetWorldLocation(Data.CurrentLocation);
			Data.NiagaraComponent->SetWorldRotation(MoveDirection.Rotation());
		}
	}
}

void ABaseTowerNice::PlayMuzzleFlashEffect(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (MuzzleFlashNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),MuzzleFlashNiagaraSystem, SpawnLocation, SpawnRotation);
	}
}

void ABaseTowerNice::PlayHitEffect(const FVector& HitLocation)
{
	if (HitNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),HitNiagaraSystem, HitLocation,FRotator::ZeroRotator);
	}
}



