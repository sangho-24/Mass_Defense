#include "Actor/BaseMonster.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Actor/SplinePathActor.h"

ABaseMonster::ABaseMonster()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	
}

void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();
	
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

void ABaseMonster::TickMoveAlongSpline(float DeltaTime)
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

void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickMoveAlongSpline(DeltaTime);
}

void ABaseMonster::SetSplinePathActor(ASplinePathActor* InSplinePathActor)
{
	SplinePathActor = InSplinePathActor; 
}

void ABaseMonster::SetPathOffset(float InPathOffset)
{
	PathOffset = InPathOffset;
}

float ABaseMonster::ApplyDamage(float InDamageAmount, AActor* InAttacker)
{
	if (bIsDead || InDamageAmount <= 0.0f)  return 0.0f;
	float AppliedDamage = InDamageAmount;
	CurrentHealth -= AppliedDamage;
	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		AppliedDamage += CurrentHealth; // 실제 적용된 데미지 계산
		CurrentHealth = 0.0f;
		Death(InAttacker);
	}
	return AppliedDamage;
}

void ABaseMonster::Death(AActor* InKiller)
{
	bIsDead = true;
	SetActorTickEnabled(false);
	// TODO: 몬스터 사망 시 처리 로직 추가 (예: 애니메이션 재생, 점수 증가)
	Destroy();
}
