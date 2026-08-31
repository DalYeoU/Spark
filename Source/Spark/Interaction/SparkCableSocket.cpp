#include "Interaction/SparkCableSocket.h"
#include "Interaction/SparkCablePlug.h"
#include "Interaction/SparkDoor.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"

ASparkCableSocket::ASparkCableSocket()
{
    PrimaryActorTick.bCanEverTick = false;

    // 플러그가 스냅되어 고정될 위치 기준점 컴포넌트 생성
    SocketAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SocketAttachPoint"));
    SetRootComponent(SocketAttachPoint);
}

void ASparkCableSocket::BeginPlay()
{
    Super::BeginPlay();
}

bool ASparkCableSocket::CanInteract_Implementation(APawn* InstigatorPawn) const
{
    // 이미 전력이 들어와 있는 소켓이면 상호작용 불필요
    if (bIsPowered)
    {
        return false;
    }

    if (!InstigatorPawn)
    {
        return false;
    }

    // 플레이어가 플러그를 자식 액터로 들고 있는지 검사
    TArray<AActor*> AttachedActors;
    InstigatorPawn->GetAttachedActors(AttachedActors);

    for (AActor* AttachedActor : AttachedActors)
    {
        if (Cast<ASparkCablePlug>(AttachedActor))
        {
            return true;
        }
    }

    return false;
}

void ASparkCableSocket::Interact_Implementation(APawn* InstigatorPawn)
{
    if (!InstigatorPawn || bIsPowered)
    {
        return;
    }

    // 플레이어가 들고 있는 플러그 액터 탐색
    TArray<AActor*> AttachedActors;
    InstigatorPawn->GetAttachedActors(AttachedActors);

    for (AActor* AttachedActor : AttachedActors)
    {
        if (ASparkCablePlug* Plug = Cast<ASparkCablePlug>(AttachedActor))
        {
            // 플러그를 이 소켓에 꽂도록 명령
            Plug->PlugIntoSocket(this);
            break;
        }
    }
}

FText ASparkCableSocket::GetInteractionText_Implementation() const
{
    if (bIsPowered)
    {
        return FText::FromString(TEXT("전력 공급 중"));
    }

    return FText::FromString(TEXT("E 키를 눌러 케이블 연결"));
}

void ASparkCableSocket::PlugIn(ASparkCablePlug* InPlug)
{
    if (!InPlug || bIsPowered)
    {
        return;
    }

    bIsPowered = true;
    ConnectedPlug = InPlug;

    // 블루프린트 연출 트리거 (빛/소리)
    BP_OnPowerRestored();

    // 연결된 문 열기
    if (TargetDoor)
    {
        TargetDoor->OpenDoor();
    }
}

void ASparkCableSocket::Unplug()
{
    if (!bIsPowered)
    {
        return;
    }

    bIsPowered = false;
    ConnectedPlug = nullptr;

    // 블루프린트 연출 트리거
    BP_OnPowerBlocked();

    // 연결된 문 닫기
    if (TargetDoor)
    {
        TargetDoor->CloseDoor();
    }
}
