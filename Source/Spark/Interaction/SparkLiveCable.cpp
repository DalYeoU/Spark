#include "Interaction/SparkLiveCable.h"

#include "Components/BoxComponent.h"
#include "Components/SparkComponent.h"
#include "Character/SparkCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASparkLiveCable::ASparkLiveCable()
{
    PrimaryActorTick.bCanEverTick = false;

    SparkTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("SparkTrigger"));
    RootComponent = SparkTrigger;
    SparkTrigger->SetBoxExtent(FVector(40.0f, 40.0f, 150.0f));

    // 폰 오버랩만 감지한다. Wall 트레이스 채널을 포함한 나머지는 무시하므로 벽 슬라이드가 걸리거나 길을 막지 않는다
    SparkTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SparkTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    SparkTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    SparkTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASparkLiveCable::BeginPlay()
{
    Super::BeginPlay();

    SparkTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASparkLiveCable::OnOverlapBegin);
    SparkTrigger->OnComponentEndOverlap.AddDynamic(this, &ASparkLiveCable::OnOverlapEnd);
}

void ASparkLiveCable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_Repeat);
    }

    Super::EndPlay(EndPlayReason);
}

void ASparkLiveCable::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ASparkCharacter* Character = Cast<ASparkCharacter>(OtherActor);
    if (!Character) return;

    // 한 캐릭터의 여러 컴포넌트가 겹쳐도 타이머가 중복으로 걸리지 않게 한다
    if (OverlappingCharacter.Get() == Character) return;

    OverlappingCharacter = Character;
    EmitSpark();
    GetWorldTimerManager().SetTimer(TimerHandle_Repeat, this, &ASparkLiveCable::EmitSpark, RepeatInterval, true);
}

void ASparkLiveCable::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor != OverlappingCharacter.Get()) return;

    OverlappingCharacter.Reset();
    GetWorldTimerManager().ClearTimer(TimerHandle_Repeat);
}

void ASparkLiveCable::EmitSpark()
{
    const ASparkCharacter* Character = OverlappingCharacter.Get();
    if (!Character) return;

    USparkComponent* SparkComponent = Character->FindComponentByClass<USparkComponent>();
    if (!SparkComponent) return;

    // 캐릭터와 가장 가까운 박스 표면 점을 접촉 지점으로 쓴다. 캐릭터 중심이 박스 안이면 그 위치를 그대로 쓴다
    const FVector CharacterLocation = Character->GetActorLocation();
    FVector ContactPoint = CharacterLocation;
    SparkTrigger->GetClosestPointOnCollision(CharacterLocation, ContactPoint);

    FVector Normal = (CharacterLocation - ContactPoint).GetSafeNormal();
    if (Normal.IsNearlyZero())
    {
        Normal = FVector::UpVector;
    }

    SparkComponent->TriggerCableSpark(ContactPoint, Normal);
}
