#include "ProjectilePoolWorldSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void UProjectilePoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ProjectilePool.Reserve(8);
}

void UProjectilePoolWorldSubsystem::Deinitialize()
{
	for (auto& Tuple : ProjectilePool)
	{
		FProjectilePool& Pool = Tuple.Value;
		for (TObjectPtr<UNiagaraComponent>& Component : Pool.ProjectileComponentArray)
		{
			if (IsValid(Component.Get()))
			{
				Component->DestroyComponent();
			}
		}
		Pool.ProjectileComponentArray.Empty();
	}
	ProjectilePool.Empty();
	Super::Deinitialize();
}

void UProjectilePoolWorldSubsystem::PrewarmPool(UNiagaraSystem* InTemplateSystem, int32 PrewarmCount)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(InTemplateSystem))
	{
		return;
	}
	FProjectilePool& Pool = ProjectilePool.FindOrAdd(InTemplateSystem);

	const int32 TargetCount = FMath::Clamp(PrewarmCount, 0, MaxPoolSize);
	const int32 RequiredCount = TargetCount - Pool.ProjectileComponentArray.Num();

	if (RequiredCount <= 0)
	{
		return;
	}
	Pool.ProjectileComponentArray.Reserve(TargetCount);
	for (int32 i = 0; i < RequiredCount; ++i)
	{
		UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, InTemplateSystem,FVector::ZeroVector,FRotator::ZeroRotator,FVector::OneVector,
				false, false, ENCPoolMethod::None, false);

		if (!IsValid(NewComponent))
		{
			continue;
		}
		NewComponent->DeactivateImmediate();
		NewComponent->SetVisibility(false);
		Pool.ProjectileComponentArray.Add(NewComponent);
	}
}

UNiagaraComponent* UProjectilePoolWorldSubsystem::AcquireProjectile(UNiagaraSystem* InTemplateSystem,
	const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(InTemplateSystem))
	{
		return nullptr;
	}
	FProjectilePool* Pool = ProjectilePool.Find(InTemplateSystem);
	UNiagaraComponent* ResultComponent = nullptr;
	// 풀에 사용 가능한 컴포넌트가 있으면 재사용
	if (Pool && Pool->ProjectileComponentArray.Num() > 0)
	{
		ResultComponent = Pool->ProjectileComponentArray.Pop(EAllowShrinking::No);
	}
	// 풀이 비어 있으면 동적 생성
	else if (UWorld* World = GetWorld())
	{
		ResultComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, InTemplateSystem, SpawnLocation, SpawnRotation, FVector::OneVector, 
				false, false, ENCPoolMethod::None, false);
	}

	if (!IsValid(ResultComponent))
	{
		return nullptr;
	}

	ResultComponent->SetWorldLocationAndRotation(SpawnLocation, SpawnRotation);
	ResultComponent->SetVisibility(true);
	ResultComponent->ResetSystem();
	ResultComponent->Activate(true);

	return ResultComponent;
}

void UProjectilePoolWorldSubsystem::ReturnProjectile(UNiagaraComponent* ComponentToReturn)
{
	if (!IsValid(ComponentToReturn))
	{
		return;
	}
	UNiagaraSystem* NiagaraSystem = ComponentToReturn->GetAsset();
	if (!IsValid(NiagaraSystem))
	{
		ComponentToReturn->DestroyComponent();
		return;
	}

	ComponentToReturn->DeactivateImmediate();
	ComponentToReturn->SetVisibility(false);
	FProjectilePool& Pool = ProjectilePool.FindOrAdd(NiagaraSystem);
	
	if (Pool.ProjectileComponentArray.Num() >= MaxPoolSize)
	{
		ComponentToReturn->DestroyComponent();
		return;
	}
	Pool.ProjectileComponentArray.Add(ComponentToReturn);
}