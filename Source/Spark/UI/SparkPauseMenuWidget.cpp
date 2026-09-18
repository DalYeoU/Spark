#include "UI/SparkPauseMenuWidget.h"

#include "Character/SparkCharacter.h"
#include "Framework/SparkPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void USparkPauseMenuWidget::OnResumeClicked()
{
    if (ASparkPlayerController* SparkPlayerController = Cast<ASparkPlayerController>(GetOwningPlayer()))
    {
        SparkPlayerController->SetPauseMenuVisible(false);
    }
}

void USparkPauseMenuWidget::OnRestartClicked()
{
    BP_ShowRestartConfirm();
}

void USparkPauseMenuWidget::OnRestartConfirmed()
{
    BP_HideRestartConfirm();

    if (ASparkCharacter* SparkCharacter = Cast<ASparkCharacter>(GetOwningPlayerPawn()))
    {
        SparkCharacter->RestartLevelFromCheckpoint();
    }
}

void USparkPauseMenuWidget::OnRestartCancelled()
{
    BP_HideRestartConfirm();
}

void USparkPauseMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
