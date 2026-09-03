#include "Interaction/SparkSwitch.h"
#include "Interaction/SparkDoor.h"

ASparkSwitch::ASparkSwitch()
{
    // 스위치도 매 프레임 업데이트할 틱이 필요 없으므로 false 설정
    PrimaryActorTick.bCanEverTick = false;

    // 기본 상호작용 안내 문구 설정
    ActiveInteractionText = FText::FromString(TEXT("E 키를 눌러 장치 활성화"));
    InactiveInteractionText = FText::FromString(TEXT("이미 활성화됨"));
}

void ASparkSwitch::BeginPlay()
{
    Super::BeginPlay();
}

bool ASparkSwitch::CanInteract_Implementation(APawn* InstigatorPawn) const
{
    // 토글형 스위치이면 항상 상호작용 가능
    if (bReusable)
    {
        return true;
    }

    // 일회성 스위치이면 아직 활성화되지 않았을 때만 상호작용 가능
    return !bIsActivated;
}

void ASparkSwitch::Interact_Implementation(APawn* InstigatorPawn)
{
    if (bReusable)
    {
        // 켜짐/꺼짐 상태 전환
        bIsActivated = !bIsActivated;
        if (bIsActivated)
        {
            BP_OnSwitchActivated();
            if (TargetDoor)
            {
                TargetDoor->OpenDoor();
            }
        }
        else
        {
            BP_OnSwitchDeactivated();
            if (TargetDoor)
            {
                TargetDoor->CloseDoor();
            }
        }
    }
    else
    {
        // 아직 켜지지 않은 경우에만 활성화 실행
        if (!bIsActivated)
        {
            bIsActivated = true;
            BP_OnSwitchActivated();
            if (TargetDoor)
            {
                TargetDoor->OpenDoor();
            }
        }
    }
}

FText ASparkSwitch::GetInteractionText_Implementation() const
{
    if (CanInteract_Implementation(nullptr))
    {
        return ActiveInteractionText;
    }

    return InactiveInteractionText;
}
