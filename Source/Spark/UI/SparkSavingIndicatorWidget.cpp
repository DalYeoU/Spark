#include "UI/SparkSavingIndicatorWidget.h"

#include "Components/TextBlock.h"
#include "Save/SparkSaveSubsystem.h"

void USparkSavingIndicatorWidget::NativeConstruct()
{
    Super::NativeConstruct();

    USparkSaveSubsystem::OnSaveStartedGlobal.AddDynamic(this, &USparkSavingIndicatorWidget::HandleSaveStarted);
    USparkSaveSubsystem::OnSaveCompletedGlobal.AddDynamic(this, &USparkSavingIndicatorWidget::HandleSaveCompleted);

    // 저장 중이 아닌 기본 상태에서는 숨김
    SetVisibility(ESlateVisibility::Collapsed);
}

void USparkSavingIndicatorWidget::NativeDestruct()
{
    USparkSaveSubsystem::OnSaveStartedGlobal.RemoveDynamic(this, &USparkSavingIndicatorWidget::HandleSaveStarted);
    USparkSaveSubsystem::OnSaveCompletedGlobal.RemoveDynamic(this, &USparkSavingIndicatorWidget::HandleSaveCompleted);

    Super::NativeDestruct();
}

void USparkSavingIndicatorWidget::HandleSaveStarted()
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("저장 중...")));
    }

    // Collapsed 상태로는 애니메이션이 재생돼도 렌더링되지 않으므로 재생 전 Visible로 전환
    SetVisibility(ESlateVisibility::HitTestInvisible);
    BP_OnShowSaving();
}

void USparkSavingIndicatorWidget::HandleSaveCompleted(bool bSuccess)
{
    if (StatusText)
    {
        StatusText->SetText(bSuccess ? FText::FromString(TEXT("저장 완료")) : FText::FromString(TEXT("저장 실패")));
    }

    BP_OnSaveFinished(bSuccess);
}
