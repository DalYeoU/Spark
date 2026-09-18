#include "UI/SparkFailureMessageWidget.h"

#include "Components/TextBlock.h"

void USparkFailureMessageWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::Collapsed);
}

void USparkFailureMessageWidget::ShowMessage(const FText& Message)
{
    if (MessageText)
    {
        MessageText->SetText(Message);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);
    BP_OnShowMessage();
}

void USparkFailureMessageWidget::HideMessage()
{
    SetVisibility(ESlateVisibility::Collapsed);
    BP_OnHideMessage();
}
