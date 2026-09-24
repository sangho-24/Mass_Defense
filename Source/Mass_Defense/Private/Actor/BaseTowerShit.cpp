// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BaseTowerShit.h"
#include "Components/CapsuleComponent.h"
#include "Actor/BaseProjectile.h"
#include "Interface/DamageableInterface.h"
#include "Kismet/GameplayStatics.h"

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
	
	GetWorld()->GetTimerManager().SetTimer(
	AttackTimerHandle,this, 
	&ABaseTowerShit::AttackTimer,
	AttackInterval,true);
}

void ABaseTowerShit::AttackTimer()
{
	if (!IsTargetValid())
	{
	UE_LOG(LogTemp, Log, TEXT("타겟이 유효하지 않음. 새로운 타겟 탐색"));
		CurrentTarget = FindNearestTarget();
	}
	if (CurrentTarget.IsValid())
	{
		FireAtTarget(CurrentTarget.Get());
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

// Called every frame
void ABaseTowerShit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

