
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolWorldSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// UHT에서는 언리얼 자료구조 중첩 허용하지 않으므로 구조체로
USTRUCT()
struct FProjectilePool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ProjectileComponentArray;
};

UCLASS()
class MASS_DEFENSE_API UProjectilePoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
private:
	// 재사용 대기 중인 Niagara Component 풀
	UPROPERTY()
	TMap<TObjectPtr<UNiagaraSystem>, FProjectilePool> ProjectilePool;

	static constexpr int32 MaxPoolSize = 8000;
	
public:
	// UWorldSubsystem 인터페이스 (월드 생성 및 파괴 시 자동 호출)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Optimization|SetUp")
	void PrewarmPool(UNiagaraSystem* InTemplateSystem, int32 PrewarmCount = 2000);
	
	UNiagaraComponent* AcquireProjectile(UNiagaraSystem* InTemplateSystem, const FVector& SpawnLocation, const FRotator& SpawnRotation);
	void ReturnProjectile(UNiagaraComponent* ComponentToReturn);
	
};
