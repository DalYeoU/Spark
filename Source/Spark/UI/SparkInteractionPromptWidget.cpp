#include "UI/SparkInteractionPromptWidget.h"

#include "Components/TextBlock.h"
#include "Components/SparkInteractionComponent.h"
#include "Character/SparkCharacter.h"
#include "Interaction/Interactable.h"

void USparkInteractionPromptWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 초기 생성 상태에서는 상호작용 대상이 없으므로 화면에서 숨김 처리
    SetVisibility(ESlateVisibility::Collapsed);
    
    // 플레이어 폰을 가져와 ASparkCharacter 및 USparkInteractionComponent 탐색
    if (APawn* OwningPawn = GetOwningPlayerPawn())
    {
        if (ASparkCharacter* SparkCharacter = Cast<ASparkCharacter>(OwningPawn))
        {
            if (USparkInteractionComponent* InteractionComp = SparkCharacter->GetInteractionComponent())
            {
                CachedInteractionComponent = InteractionComp;

                // 상호작용 대상 변경 이벤트(OnInteractionTargetChanged)에 핸들러 함수 바인딩
                InteractionComp->OnInteractionTargetChanged.AddDynamic(this, &USparkInteractionPromptWidget::HandleInteractionTargetChanged);
                
                // 위젯 생성 시점에 이미 대상이 포커스되어 있는 경우를 대비해 초기 갱신 실행
                HandleInteractionTargetChanged(InteractionComp->GetCurrentInteractableActor());
            }
        }
    }
}

void USparkInteractionPromptWidget::NativeDestruct()
{
    // 위젯 파괴 시 댕글링 포인터 및 메모리 누수를 방지하기 위해 델리게이트 안전 해제
    if (CachedInteractionComponent.IsValid())
    {
        CachedInteractionComponent->OnInteractionTargetChanged.RemoveDynamic(this, &USparkInteractionPromptWidget::HandleInteractionTargetChanged);
    }
    
    Super::NativeDestruct();
}

void USparkInteractionPromptWidget::HandleInteractionTargetChanged(AActor* NewTarget)
{
    // 새로운 상호작용 대상이 유효하고 IInteractable 인터페이스를 구현하고 있는 경우
    if (NewTarget && NewTarget->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        // 대상 액터로부터 프롬프트에 표시할 로컬라이즈 텍스트 추출
        const FText PromptText = IInteractable::Execute_GetInteractionText(NewTarget);
        
        if (InteractionText)
        {
            InteractionText->SetText(PromptText);
        }
        
        // 프롬프트 UI 표시 및 블루프린트 연출/애니메이션 트리거
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        BP_OnShowPrompt(PromptText);
    }
    else
    {
        // 대상에서 벗어났거나 유효하지 않은 경우 프롬프트 숨김
        SetVisibility(ESlateVisibility::Collapsed);
        BP_OnHidePrompt();
    }
}
