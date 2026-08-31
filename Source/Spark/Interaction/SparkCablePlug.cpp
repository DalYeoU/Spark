#include "Interaction/SparkCablePlug.h"
#include "Interaction/SparkCableSocket.h"
#include "CableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"

ASparkCablePlug::ASparkCablePlug()
{
    PrimaryActorTick.bCanEverTick = false;

    PlugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlugMesh"));
    SetRootComponent(PlugMesh);
    PlugMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
    CableComponent->SetupAttachment(RootComponent);
    CableComponent->CableLength = 300.0f;
    CableComponent->NumSegments = 10;
    CableComponent->SolverIterations = 4;
}

void ASparkCablePlug::BeginPlay()
{
    Super::BeginPlay();
}

bool ASparkCablePlug::CanInteract_Implementation(APawn* InstigatorPawn) const
{
    // 이미 소켓에 꽂혀 있거나 현재 들고 있는 상태라면 바닥에서 다시 집을 수 없음
    if (AttachedSocket.IsValid() || HoldingPawn.IsValid())
    {
        return false;
    }

    return true;
}

void ASparkCablePlug::Interact_Implementation(APawn* InstigatorPawn)
{
    // 바닥에 놓여 있는 상태에서 E키를 누르면 집기
    if (!HoldingPawn.IsValid() && !AttachedSocket.IsValid())
    {
        PickUp(InstigatorPawn);
    }
}

FText ASparkCablePlug::GetInteractionText_Implementation() const
{
    if (AttachedSocket.IsValid())
    {
        return FText::FromString(TEXT("케이블 연결 완료"));
    }

    return FText::FromString(TEXT("E 키를 눌러 케이블 집기"));
}

void ASparkCablePlug::PickUp(APawn* InstigatorPawn)
{
    if (!InstigatorPawn) return;

    HoldingPawn = InstigatorPawn;
    PlugMesh->SetSimulatePhysics(false);
    PlugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 플레이어의 전방 60cm 위치에 들기
    AttachToActor(InstigatorPawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    SetActorRelativeLocation(FVector(60.0f, 0.0f, 0.0f));
}

void ASparkCablePlug::Drop()
{
    HoldingPawn = nullptr;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    PlugMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ASparkCablePlug::PlugIntoSocket(ASparkCableSocket* TargetSocket)
{
    if (!TargetSocket) return;

    HoldingPawn = nullptr;
    AttachedSocket = TargetSocket;

    // 플레이어에게서 분리
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 소켓의 AttachPoint에 착 달라붙도록 스냅
    if (USceneComponent* AttachPoint = TargetSocket->GetSocketAttachPoint())
    {
        AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        AttachToActor(TargetSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }

    SetActorRelativeLocation(FVector::ZeroVector);
    SetActorRelativeRotation(FRotator::ZeroRotator);
    PlugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 소켓에게 결합 신호 전달
    TargetSocket->PlugIn(this);
}

ASparkCableSocket* ASparkCablePlug::FindNearbySocket() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    const FVector MyLoc = GetActorLocation();

    // 주변 액터 중 ASparkCableSocket 검색
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(SocketSnapDistance);
    FCollisionQueryParams QueryParams(FName(TEXT("SocketFind")), false, this);

    if (World->OverlapMultiByChannel(Overlaps, MyLoc, FQuat::Identity, ECC_WorldDynamic, Sphere, QueryParams))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (ASparkCableSocket* Socket = Cast<ASparkCableSocket>(Overlap.GetActor()))
            {
                if (!Socket->IsPowered())
                {
                    return Socket;
                }
            }
        }
    }

    return nullptr;
}
