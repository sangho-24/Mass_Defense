#include "Actor/SplinePathActor.h"
#include "Components/SplineComponent.h"


ASplinePathActor::ASplinePathActor()
{
	// 필요가 생기면 켜주자.
	PrimaryActorTick.bCanEverTick = false;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;

	// 필요에 따라 기본 스플라인 포인트를 초기화하거나 루프 여부를 C++에서 설정할 수 있습니다.
	// SplineComponent->SetClosedLoop(false);

}

void ASplinePathActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplinePathActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

