// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BaseTowerShit.h"
#include "Components/CapsuleComponent.h"
#include "Interface/DamageableInterface.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseTowerShit::ABaseTowerShit()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
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
	AActor* BestTarget = FindNearestTarget();
	if (BestTarget)
	{
		FireAtTarget(BestTarget);
	}
}

AActor* ABaseTowerShit::FindNearestTarget()
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
	if (!TargetActor)
	{
		return;
	}
	if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(TargetActor))
	{
		Damageable->ApplyDamage(AttackDamage, this);
	}
}

// Called every frame
void ABaseTowerShit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

