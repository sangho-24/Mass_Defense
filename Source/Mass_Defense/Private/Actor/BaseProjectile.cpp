
#include "Actor/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/DamageableInterface.h"
#include "Actor/MonsterSpawnManagerBatch.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetupAttachment(RootComponent);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	
	InitialLifeSpan = 5.0f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (CollisionComponent)
	{
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlap);
	}
}

void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasHit || bUseISM || !OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	// 유도 타겟팅 로직
	if (ProjectileMovement && ProjectileMovement->bIsHomingProjectile)
	{
		// 현재 타겟이 메모리에 살아있고, 게임 논리적으로도 생존 상태인지 검증
		bool bIsTargetAlive = false;
		if (CurrentTarget.IsValid())
		{
			if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(CurrentTarget.Get()))
			{
				bIsTargetAlive = !Damageable->IsDead();
			}
		}
	
		
		if (bIsTargetAlive)
		{
			// 타겟이 살아있다면 다른 대상은 무시
			if (OtherActor != CurrentTarget.Get())
			{
				return;
			}
		}
		else
		{
			// 날아가던 중 타겟이 죽었다면 유도를 비활성화
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->HomingTargetComponent = nullptr;
			CurrentTarget = nullptr;
			// 이제부터 이 투사체가 아무 몬스터나 공격 가능
		}
	}
	
	if (IDamageableInterface* Damageable = Cast<IDamageableInterface>(OtherActor))
	{
		if (Damageable->IsDead())
		{
			return;
		}
		bHasHit = true;
		Damageable->ApplyDamage(Damage, GetOwner());
	}
	// 유도 상태 아닐 때, 인터페이스 없는 대상(바닥, 장식 등) 맞아도 파괴. 싫으면 위로 올리셈.
	Destroy();
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUseISM)
	{
		TickISMTravel(DeltaTime);
	}
}

void ABaseProjectile::SetHomingTarget(AActor* InTargetActor)
{
	if (!InTargetActor || !ProjectileMovement)
	{
		return;
	}
	
	bUseISM = false;
	TargetSpawnManager = nullptr;
	TargetISMIndex = INDEX_NONE;
	CurrentTarget = InTargetActor;
	
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 20000.0f;
	ProjectileMovement->HomingTargetComponent = InTargetActor->GetRootComponent();
}

void ABaseProjectile::SetISMTarget(AMonsterSpawnManagerBatch* InManager, int32 InTargetIndex)
{
	TargetSpawnManager = InManager;
	TargetISMIndex = InTargetIndex;
	bUseISM = true;
	CurrentTarget = nullptr;
	// ISM 인스턴스는 씬 컴포넌트가 없으므로 내장 물리 유도 기능은 비활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
	}
}

void ABaseProjectile::TickISMTravel(float DeltaTime)
{
	if (bHasHit || !TargetSpawnManager || TargetISMIndex == INDEX_NONE)
	{
		return;
	}
	// 매니저를 통해 해당 인스턴스 위치 조회
	const FVector TargetLocation = TargetSpawnManager->GetMonsterLocation(TargetISMIndex);
	if (TargetLocation.IsZero())
	{
		// 유도를 중단하고 현재 속도 벡터 그대로 직진하게 둠
		TargetISMIndex = INDEX_NONE;
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceSq = ToTarget.SizeSquared();
	// 도달 판정 (거리 제곱 비교)
	if (DistanceSq <= FMath::Square(HitAcceptanceRadius))
	{
		bHasHit = true;
		// 매니저의 인스턴스에 데미지 적용
		TargetSpawnManager->ApplyDamageToInstance(TargetISMIndex, Damage, GetOwner());
		Destroy();
		return;
	}

	// 실시간 궤적 수정
	const FVector Direction = ToTarget.GetSafeNormal();
	const float CurrentSpeed = ProjectileMovement ? ProjectileMovement->InitialSpeed : 1000.0f;
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * CurrentSpeed;
	}
	SetActorRotation(Direction.Rotation());
}


