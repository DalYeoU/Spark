#include "Interaction/SparkHazardZone.h"
#include "Components/BoxComponent.h"
#include "Character/SparkCharacter.h"

ASparkHazardZone::ASparkHazardZone()
{
	PrimaryActorTick.bCanEverTick = true;
    
    // 트리거 박스 컴포넌트 생성 및 루트 지정
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    
    // 오버랩 전용 Trigger 프로필 지정
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ASparkHazardZone::BeginPlay()
{
	Super::BeginPlay();
    
    // 오버랩 동적 이벤트 바인딩
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASparkHazardZone::OnOverlapBegin);
}

void ASparkHazardZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 진입한 대상이 플레이어 캐릭터라면 사망/리스폰 처리
    if (ASparkCharacter* SparkCharacter = Cast<ASparkCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("SparkCharacter Overlapped HazardZone %s"), *GetName());
        
        // 캐릭터에게 FellOutOfWorld(사망/리스폰) 호출
        
        SparkCharacter->FellOutOfWorld(*GetDefault<UDamageType>());
    }
}

