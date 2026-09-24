// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MonsterSpawnManager.h"
#include "Actor/BaseMonster.h"
#include "Actor/SplinePathActor.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMonsterSpawnManager::AMonsterSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AMonsterSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (!SplinePathActor || !SplinePathActor->GetSplineComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("경로 스플라인 액터가 없슴다"));
		return;
	}
	CachedSpawnTransform = SplinePathActor->GetSplineComponent()->GetTransformAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("스폰할 몬스터 클래스가 없슴다"));
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,this, 
		&AMonsterSpawnManager::SpawnMonster,
		SpawnInterval,true);
}

void AMonsterSpawnManager::SpawnMonster()
{
	// 모두 스폰했다면 타이머를 정리하고 종료
	if (CurrentSpawnedCount >= TotalSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("총 %d 마리 스폰 완료!"), CurrentSpawnedCount);
		return;
	}
	
	// 지연 생성(메모리 할당 및 기본 생성자만 불린 상태로 대기) 시작
	// 몬스터의 BeginPlay 아직 실행 안됨
	ABaseMonster* NewMonster = GetWorld()->SpawnActorDeferred<ABaseMonster>(
		MonsterClass, CachedSpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	if (NewMonster)
	{
		float Offset = FMath::RandRange(-RandomOffset, RandomOffset);
		NewMonster->SetPathOffset(Offset);
		NewMonster->SetSplinePathActor(SplinePathActor);
		// 스플라인 설정 후 스폰 마무리 (이 순간 몬스터의 BeginPlay가 호출됨)
		UGameplayStatics::FinishSpawningActor(NewMonster, CachedSpawnTransform);

		CurrentSpawnedCount+=1;
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
}


