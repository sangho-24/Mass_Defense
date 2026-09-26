#include "Actor/BaseMonsterShit.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Actor/SplinePathActor.h"

ABaseMonsterShit::ABaseMonsterShit()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionProfileName(TEXT("DamageableMonster"));
	RootComponent = CollisionComponent;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void ABaseMonsterShit::BeginPlay()
{
	Super::BeginPlay();
	
	// 스켈레탈/스태틱 스위치
	if (bUseVAT)
	{
		if (SkeletalMeshComponent)
		{
			SkeletalMeshComponent->SetVisibility(false);
			SkeletalMeshComponent->SetComponentTickEnabled(false);
		}
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetVisibility(true);
			StaticMeshComponent->SetComponentTickEnabled(true);
		}
	}
	else
	{
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetVisibility(false);
			StaticMeshComponent->SetComponentTickEnabled(false);
		}
		if (SkeletalMeshComponent)
		{
			SkeletalMeshComponent->SetVisibility(true);
			SkeletalMeshComponent->SetComponentTickEnabled(true);
		}
	}
	// 임시 랜덤상수. ISM로 전환하면 제거
	StaticMeshComponent->SetCustomPrimitiveDataFloat(0, 1.f);
	StaticMeshComponent->SetCustomPrimitiveDataFloat(1, FMath::FRandRange(0.f, 1.f));
	StaticMeshComponent->SetCustomPrimitiveDataFloat(2, 0.f);
	StaticMeshComponent->SetCustomPrimitiveDataFloat(3, 30.f);



	CurrentHealth = MaxHealth;
	// 스플라인 액터로부터 컴포넌트를 다이렉트로 가져와 캐싱.
	if (SplinePathActor)
	{
		CachedSplineComponent = SplinePathActor->GetSplineComponent();
		if (CachedSplineComponent)
		{
			CachedSplineLength = CachedSplineComponent->GetSplineLength();
		}
	}

	// 경로 정보가 유효하지 않다면 매 프레임 틱을 돌며 낭비할 이유가 없으므로 틱을 차단.
	if (CachedSplineComponent == nullptr || CachedSplineLength <= 0.0f)
	{
		SetActorTickEnabled(false);
	}
}

void ABaseMonsterShit::TickMoveAlongSpline(float DeltaTime)
{
	CurrentSplineDistance += MoveSpeed * DeltaTime;
	if (CurrentSplineDistance > CachedSplineLength)
	{
		CurrentSplineDistance = CachedSplineLength; // 끝에 도달하면 멈춤
		SetActorTickEnabled(false);
	}
	FTransform  NewTransform = CachedSplineComponent->GetTransformAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
	FVector Location = NewTransform.GetLocation();
	FVector Right = NewTransform.GetRotation().GetRightVector();
	Location += Right * PathOffset;
	
	SetActorLocationAndRotation(Location, NewTransform.GetRotation(),true);
}

void ABaseMonsterShit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickMoveAlongSpline(DeltaTime);
}

void ABaseMonsterShit::SetSplinePathActor(ASplinePathActor* InSplinePathActor)
{
	SplinePathActor = InSplinePathActor; 
}

void ABaseMonsterShit::SetPathOffset(float InPathOffset)
{
	PathOffset = InPathOffset;
}

float ABaseMonsterShit::ApplyDamage(float InDamageAmount, AActor* InAttacker)
{
	if (bIsDead || InDamageAmount <= 0.f)  
		return 0.f;
	float AppliedDamage = InDamageAmount;
	CurrentHealth -= AppliedDamage;
	if (CurrentHealth <= 0.f && !bIsDead)
	{
		AppliedDamage += CurrentHealth; // 실제 적용된 데미지 계산
		CurrentHealth = 0.f;
		Death(InAttacker);
	}
	return AppliedDamage;
}

void ABaseMonsterShit::Death(AActor* InKiller)
{
	bIsDead = true;
	SetActorTickEnabled(false);
	if (bUseVAT)
	{
		StaticMeshComponent->SetCustomPrimitiveDataFloat(0,0.f);
		StaticMeshComponent->SetCustomPrimitiveDataFloat(1, -GetWorld()->GetTimeSeconds());
		UE_LOG(LogTemp, Log, TEXT("타임: %f"), GetWorld()->GetTimeSeconds());
		StaticMeshComponent->SetCustomPrimitiveDataFloat(2,31.f);
		StaticMeshComponent->SetCustomPrimitiveDataFloat(3,56.f);
	}
	else
    {
		if (DeathAnimAsset)
			SkeletalMeshComponent->PlayAnimation(DeathAnimAsset, false);
    }
	// TODO: 오브젝트 풀링 고려해보기
	SetLifeSpan(3.0f);
	// TODO: 몬스터 사망 시 처리 로직 추가 (점수 증가, 충돌 제거)
}
