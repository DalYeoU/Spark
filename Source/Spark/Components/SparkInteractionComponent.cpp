#include "Components/SparkInteractionComponent.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Interaction/Interactable.h"

USparkInteractionComponent::USparkInteractionComponent()
{
    // 탐지는 타이머로 돌리니까 틱은 꺼둠
    PrimaryComponentTick.bCanEverTick = false;
}

void USparkInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 0.1초마다 전방 탐지를 반복 실행
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TimerHandle_InteractionCheck,
            this,
            &USparkInteractionComponent::PerformInteractionCheck,
            CheckInterval,
            true
        );
    }
}

void USparkInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 종료 시 탐지 타이머 해제
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_InteractionCheck);
    }

    Super::EndPlay(EndPlayReason);
}

void USparkInteractionComponent::PrimaryInteract()
{
    AActor* TargetActor = CurrentInteractableActor.Get();
    if (!TargetActor)
    {
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());

    // C++/블루프린트 오버라이드를 둘 다 지원하는 Execute_ 래퍼로 호출
    if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        if (IInteractable::Execute_CanInteract(TargetActor, OwnerPawn))
        {
            IInteractable::Execute_Interact(TargetActor, OwnerPawn);
        }
    }
}

AActor* USparkInteractionComponent::GetCurrentInteractableActor() const
{
    return CurrentInteractableActor.Get();
}

void USparkInteractionComponent::PerformInteractionCheck()
{
    AActor* NewTarget = FindBestInteractable();

    // 대상이 바뀐 경우에만 갱신하고 델리게이트 브로드캐스트
    if (NewTarget != CurrentInteractableActor.Get())
    {
        CurrentInteractableActor = NewTarget;
        OnInteractionTargetChanged.Broadcast(NewTarget);
    }
}

AActor* USparkInteractionComponent::FindBestInteractable() const
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    
    if (!Owner || !World)
    {
        return nullptr;
    }

    const FVector Origin = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();

    // 구체 중심을 캡슐 중심보다 살짝 낮춰 앞에 두고 반지름을 크게 잡아, 바닥에 놓인 물체부터 가슴 높이 물체까지 한 번에 겹치게 한다
    const FVector Center = Origin + (Forward * InteractionDistance) + FVector(0.0f, 0.0f, -20.0f);

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams(FName(TEXT("InteractionOverlap")), false, Owner);
    if (!World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(InteractionRadius), QueryParams))
    {
        return nullptr;
    }

    APawn* Pawn = Cast<APawn>(Owner);
    AActor* BestActor = nullptr;

    // 정면 60도 이내(cos60=0.5)만 허용하고, 그 안에서 시선 방향에 가장 가까운 대상을 고른다
    float BestDot = 0.5f;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Candidate = Overlap.GetActor();
        if (!Candidate || !Candidate->GetClass()->ImplementsInterface(UInteractable::StaticClass())) continue;

        // 정면 벡터와 대상 방향 벡터의 내적으로 시선 각도 체크
        const FVector DirToTarget = (Candidate->GetActorLocation() - Origin).GetSafeNormal2D();
        const float Dot = FVector::DotProduct(Forward.GetSafeNormal2D(), DirToTarget);
        if (Dot < BestDot) continue;

        if (!IInteractable::Execute_CanInteract(Candidate, Pawn)) continue;

        // 벽 너머에 있는 대상은 제외한다. 구체는 벽을 통과해 겹치므로 대상까지 시선이 막혔는지 따로 확인
        const UPrimitiveComponent* Component = Overlap.GetComponent();
        const FVector TargetPoint = Component ? Component->Bounds.Origin : Candidate->GetActorLocation();
        FHitResult SightHit;
        if (World->LineTraceSingleByChannel(SightHit, Origin, TargetPoint, ECC_Visibility, QueryParams) && SightHit.GetActor() != Candidate) continue;

        BestActor = Candidate;
        BestDot = Dot;
    }

    return BestActor;
}
