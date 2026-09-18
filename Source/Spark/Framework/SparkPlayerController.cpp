#include "SparkPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Character/SparkCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SparkFailureMessageWidget.h"
#include "UI/SparkPauseMenuWidget.h"

ASparkPlayerController::ASparkPlayerController()
{
}

void ASparkPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 로컬 플레이어에 대한 Enhanced Input Subsystem을 가져와 IMC_Default 활성화
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
        GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            // Priority 0으로 기본 입력 컨텍스트 등록
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // 체크포인트 알림 UI 생성 및 뷰포트 등록
    if (IsLocalController() && CheckpointNoticeWidgetClass)
    {
        if (UUserWidget* NoticeWidget = CreateWidget<UUserWidget>(this, CheckpointNoticeWidgetClass))
        {
            NoticeWidget->AddToViewport();
        }
    }

    // 세이브 인디케이터 UI 생성 및 뷰포트 등록
    if (IsLocalController() && SavingIndicatorWidgetClass)
    {
        if (UUserWidget* SavingWidget = CreateWidget<UUserWidget>(this, SavingIndicatorWidgetClass))
        {
            SavingWidget->AddToViewport();
        }
    }

    // Pause Menu UI 생성 및 뷰포트 등록 (기본은 숨김)
    if (IsLocalController() && PauseMenuWidgetClass)
    {
        PauseMenuWidgetInstance = CreateWidget<USparkPauseMenuWidget>(this, PauseMenuWidgetClass);
        if (PauseMenuWidgetInstance)
        {
            PauseMenuWidgetInstance->AddToViewport();
            PauseMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // 실패 안내 문구 UI 생성 및 뷰포트 등록 (기본은 숨김)
    if (IsLocalController() && FailureMessageWidgetClass)
    {
        FailureMessageWidgetInstance = CreateWidget<USparkFailureMessageWidget>(this, FailureMessageWidgetClass);
        if (FailureMessageWidgetInstance)
        {
            FailureMessageWidgetInstance->AddToViewport();
        }
    }
}

void ASparkPlayerController::TogglePauseMenu()
{
    SetPauseMenuVisible(!bIsPaused);
}

void ASparkPlayerController::SetPauseMenuVisible(bool bVisible)
{
    if (bIsPaused == bVisible) return;

    bIsPaused = bVisible;

    UGameplayStatics::SetGamePaused(this, bIsPaused);

    if (PauseMenuWidgetInstance)
    {
        PauseMenuWidgetInstance->SetVisibility(bIsPaused ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (bIsPaused)
    {
        FInputModeGameAndUI InputMode;
        if (PauseMenuWidgetInstance)
        {
            InputMode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());
        }
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);
    }
    else
    {
        SetInputMode(FInputModeGameOnly());
    }

    bShowMouseCursor = bIsPaused;
}

void ASparkPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 캐릭터 빙의 성공 시 입력 바인딩 진행
    if (ASparkCharacter* SparkCharacter = Cast<ASparkCharacter>(InPawn))
    {
        if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
        {
            if (MoveAction)
            {
                EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, SparkCharacter, &ASparkCharacter::Move);
            }
            if (LookAction)
            {
                EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, SparkCharacter, &ASparkCharacter::Look);
            }
            if (JumpAction)
            {
                EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, SparkCharacter, &ASparkCharacter::Jump);
                EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, SparkCharacter, &ASparkCharacter::StopJumping);
            }
            if (InteractAction)
            {
                EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, SparkCharacter, &ASparkCharacter::Interact);
            }
            if (SprintAction)
            {
                EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, SparkCharacter, &ASparkCharacter::StartSprint);
                EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, SparkCharacter, &ASparkCharacter::StopSprint);
            }
            if (SlideAction)
            {
                EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, SparkCharacter, &ASparkCharacter::StartSlide);
                EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, SparkCharacter, &ASparkCharacter::OnSlideKeyReleased);
            }
            if (PauseAction)
            {
                EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ASparkPlayerController::TogglePauseMenu);
            }
        }
    }
}

void ASparkPlayerController::OnUnPossess()
{
    // 빙의 해제 시 이전 바인딩 정리
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->ClearActionBindings();
    }

    Super::OnUnPossess();
}
