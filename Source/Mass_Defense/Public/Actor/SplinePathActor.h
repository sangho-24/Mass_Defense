#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplinePathActor.generated.h"

class USplineComponent;

// 디펜스라서 루프나 핑퐁이나 필요 없을듯.
// UENUM(BlueprintType)
// enum class EBSPatrolRouteMode : uint8
// {
// 	Loop,
// 	PingPong
// };

UCLASS()
class MASS_DEFENSE_API ASplinePathActor : public AActor
{
	GENERATED_BODY()

private:
	// UE5에서 도입된 래퍼 타입. 월드 파티션에 맞게 지연로딩을 지원, 오브젝트 참조 추적 지원(참조 뷰어나 언리얼 인사이트 등).
		// *지연로딩이란? 접근 시 메모리에 없으면 올리고 접근(오브젝트 핸들 사용)
	// 쉬핑시에 원시포인터로 컴파일되어 오버해드도 없음. -> 원시포인터로 컴파일되니까 지연로딩도 불가.
	// 쉬핑 빌드에서는 원시포인터 방식으로 로딩시에 참조 포인터들 전부 올려두고 주소 넣어준 뒤 로딩끝남. -> 실시간 비동기 로딩 하면 프레임드랍 발생하니까
	// 참고로 PIE환경은 에디터 빌드(WITH_EDITOR=1)임.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> SplineComponent;
	
public:	
	ASplinePathActor();
	// 대량의 몬스터가 경로액터 호출을 하게되니 오버해드 줄이기 위한 인라인함수
	FORCEINLINE USplineComponent* GetSplineComponent() const { return SplineComponent; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
