
#include "Actor/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/DamageableInterface.h"

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
	
	InitialLifeSpan = 3.0f;
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
	if (bHasHit || !OtherActor || OtherActor == this || OtherActor == GetOwner())
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

}

void ABaseProjectile::SetHomingTarget(AActor* InTargetActor)
{
	if (!InTargetActor || !ProjectileMovement)
	{
		return;
	}
	CurrentTarget = InTargetActor;
	
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 20000.0f;
	ProjectileMovement->HomingTargetComponent = InTargetActor->GetRootComponent();
}

