// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BaseTowerShit.h"
#include "Components/CapsuleComponent.h"
#include "Actor/BaseProjectile.h"
#include "Interface/DamageableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/MonsterSpawnManagerBatch.h"

// Sets default values
ABaseTowerShit::ABaseTowerShit()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ABaseTowerShit::BeginPlay()
{
	Super::BeginPlay();
	
	// ISM 모드인데 매니저를 안 집어넣어 줬다면 월드에서 자동 탐색
	if (bUseISM && !TargetSpawnManager)
	{
		TargetSpawnManager = Cast<AMonsterSpawnManagerBatch>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AMonsterSpawnManagerBatch::StaticClass()));
	}
	
	const float RandomFirstDelay = FMath::FRandRange(0.0f, AttackInterval);
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle,this, 
		&ABaseTowerShit::AttackTimer,AttackInterval,true, RandomFirstDelay);
}

void ABaseTowerShit::AttackTimer()
{
	// ISM 몬스터 탐색
	if (bUseISM)
	{
		if (!IsISMTargetValid())
		{
			FVector FoundLocation;
			int32 FoundIndex = INDEX_NONE;
			if (FindNearestISMTarget(FoundLocation, FoundIndex))
			{
				CurrentTargetISMIndex = FoundIndex;
				CurrentTargetISMLocation = FoundLocation;
			}
			else
			{
				CurrentTargetISMIndex = INDEX_NONE;
			}
		}
		if (CurrentTargetISMIndex != INDEX_NONE)
		{
			FireAtISMTarget(CurrentTargetISMLocation, CurrentTargetISMIndex);
		}
	}
	// 기존 액터 탐색
	else
	{
		if (!IsTargetValid())
		{
			CurrentTarget = FindNearestTarget();
		}
		if (CurrentTarget.IsValid())
		{
			FireAtTarget(CurrentTarget.Get());
		}
	}
}

AActor* ABaseTowerShit::FindNearestTarget() const
{
	// 1. 월드의 모든 몬스터를 무식하게 긁어모음 (힙 할당 + 전수 순회)
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UDamageableInterface::StaticClass(), FoundActors);

	AActor* NearestTarget = nullptr;
	float ShortestDistance = AttackRange;
	const FVector MyLocation = GetActorLocation();

	// 2. 전체 몬스터를 대상으로 무식한 유클리드 거리 전수 조사
	for (AActor* CandidateActor : FoundActors)
	{
		if (!CandidateActor)
		{
			continue;
		}

		// 인터페이스 캐스팅으로 생존 여부 확인
		IDamageableInterface* Damageable = Cast<IDamageableInterface>(CandidateActor);
		if (!Damageable || Damageable->IsDead())
		{
			continue;
		}

		// 무거운 제곱근 거리 계산
		const float Distance = FVector::Dist(MyLocation, CandidateActor->GetActorLocation());
		if (Distance <= ShortestDistance)
		{
			ShortestDistance = Distance;
			NearestTarget = CandidateActor;
		}
	}
	return NearestTarget;
}

bool ABaseTowerShit::FindNearestISMTarget(FVector& OutLocation, int32& OutIndex) const
{
	if (!TargetSpawnManager)
	{
		return false;
	}
	return TargetSpawnManager->FindTargetMonster(GetActorLocation(), AttackRange, OutLocation, OutIndex);
}



void ABaseTowerShit::FireAtTarget(AActor* TargetActor)
{
	if (!TargetActor || !ProjectileClass)
	{
		return;
	}
	const FVector FireLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FRotator FireRotation = (TargetActor->GetActorLocation() - FireLocation).Rotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	
	ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, FireLocation, FireRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->SetDamage(AttackDamage);
		if (bIsHoming)
			Projectile->SetHomingTarget(TargetActor);
	}
}

void ABaseTowerShit::FireAtISMTarget(const FVector& TargetLocation, int32 TargetIndex)
{
	if (!ProjectileClass || !TargetSpawnManager)
	{
		return;
	}

	const FVector FireLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FRotator FireRotation = (TargetLocation - FireLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	// [의도된 병목의 핵심!]
	// 대상이 ISM이라도 '물리 투사체 액터’를 월드에 직접 스폰하여
	// 액터 생성 오버헤드, UObject 힙 할당, 컴포넌트 틱 부하를 화면에 그대로 누적시킴!
	ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, FireLocation, FireRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->SetDamage(AttackDamage);
		Projectile->SetISMTarget(TargetSpawnManager, TargetIndex);
	}
}

bool ABaseTowerShit::IsTargetValid() const
{
	if (!CurrentTarget.IsValid())
	{
		return false;
	}
	AActor* TargetActor = CurrentTarget.Get();
	if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(TargetActor))
	{
		if (Damageable->IsDead())
		{
			return false;
		}
	}
	const float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	return Distance <= AttackRange;
}

bool ABaseTowerShit::IsISMTargetValid() const
{
	if (!TargetSpawnManager || CurrentTargetISMIndex == INDEX_NONE)
	{
		return false;
	}
	// 매니저로부터 해당 인덱스의 실시간 위치 확인
	const FVector TargetLocation = TargetSpawnManager->GetMonsterLocation(CurrentTargetISMIndex);
	if (TargetLocation.IsZero())
	{
		return false;
	}
	// 무거운 제곱근 거리 계산 유지
	const float Distance = FVector::Dist(GetActorLocation(), TargetLocation);
	return Distance <= AttackRange;
}

// Called every frame
void ABaseTowerShit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

