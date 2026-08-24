#include "Components/SparkInteractionComponent.h"

#include "Engine/World.h"
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

    const FVector Forward = Owner->GetActorForwardVector();

    // 시작점을 앞으로 20만큼 당겨서 캐릭터 몸통과 바로 겹치는 것 방지
    const FVector Start = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 30.0f) + (Forward * 20.0f);
    const FVector End = Start + (Forward * InteractionDistance);

    FHitResult HitResult;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRadius);
    FCollisionQueryParams QueryParams(FName(TEXT("InteractionSweep")), false, Owner);

    // 캐릭터 정면 방향으로 Sphere Sweep 수행
    if (World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, Sphere, QueryParams))
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            // 정면 벡터와 대상 방향 벡터의 내적으로 시선 각도 체크
            const FVector DirToTarget = (HitActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
            const float Dot = FVector::DotProduct(Forward.GetSafeNormal2D(), DirToTarget);

            // 정면 60도 이내(cos60=0.5)로 보고 있을 때만 상호작용 허용
            if (Dot >= 0.5f)
            {
                APawn* Pawn = Cast<APawn>(Owner);
                if (IInteractable::Execute_CanInteract(HitActor, Pawn))
                {
                    return HitActor;
                }
            }
        }
    }

    return nullptr;
}
