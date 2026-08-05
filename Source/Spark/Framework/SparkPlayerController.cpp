#include "SparkPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/SparkCharacter.h"

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
                EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, SparkCharacter,
                                                   &ASparkCharacter::Move);
            }
            if (LookAction)
            {
                EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, SparkCharacter,
                                                   &ASparkCharacter::Look);
            }
            if (JumpAction)
            {
                EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, SparkCharacter,
                                                   &ASparkCharacter::Jump);
                EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, SparkCharacter,
                                                   &ASparkCharacter::StopJumping);
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
