#include "SparkCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

ASparkCharacter::ASparkCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 마우스 카메라 시점 회전에 캐릭터 몸통이 함께 회전하지 않도록 비활성화
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 3D 플랫폼 이동 특성에 맞춰 이동 조작 방향으로 캐릭터 몸통이 자연스럽게 회전하도록 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    // 빠른 반응감과 공중 제어력을 제공하기 위한 기본 이동 스펙 초기화
    GetCharacterMovement()->JumpZVelocity = 450.0f;     // 점프 높이 조절
    GetCharacterMovement()->AirControl = 0.85f;         // 공중 제어력 (지상 대비 약 85% 반응성)
    GetCharacterMovement()->GravityScale = 1.2f;        // 중력 스케일
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;      // 최대 이동 속도
    GetCharacterMovement()->MaxAcceleration = 4096.0f;  // 입력 즉시 최대 속도에 도달하도록 가속도 상향

    // 3인칭 쿼터뷰/팔로우 시점을 위한 스프링암 컴포넌트 생성 및 시점 회전 동기화
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // 스프링암 끝에 장착되는 플레이어 실제 화면 렌더링 카메라 컴포넌트 생성
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ASparkCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ASparkCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 공중 및 Wall Slide 감지 처리
    CheckWallSlide();
}

void ASparkCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASparkCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 카메라 시선(Pitch) 기울임에 의한 수직 이동을 방지하고, 오직 지면 평면(Yaw) 회전값만 추출
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

        // 시점 기준 수평 전진(X축) 및 우측(Y축) 방향 벡터 계산
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 캐릭터 Movement Component로 최종 수평 이동 입력 전달
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASparkCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 마우스 델타값에 기반하여 플레이어 컨트롤러의 Yaw(좌우) 및 Pitch(상하) 회전 입력 업데이트
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ASparkCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    
    // 착지 이벤트 핸들러 호출
    HandleLanded(Hit);
}

void ASparkCharacter::HandleLanded(const FHitResult& Hit)
{
    // 착지 감지 확인을 위한 임시 로그 (향후 SparkComponent 연동 지점)
    FString SurfaceName = Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT("Unknown Surface");
    UE_LOG(LogTemp, Log, TEXT("SparkCharacter Landed on: %s"), *SurfaceName);
}

void ASparkCharacter::CheckWallSlide()
{
    // 캐릭터가 공중(Falling)에 떠 있는 상태가 아니면 벽 타기 해제
    if (!GetCharacterMovement()->IsFalling())
    {
        bIsWallSliding = false;
        return;
    }

    // 캐릭터 정면 방향으로 라인 트레이스를 수행하여 벽면 감지
    FVector Start = GetActorLocation();
    FVector End = Start + (GetActorForwardVector() * WallTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

    // 정면에 벽이 감지되었을 때만 Wall Slide 활성화
    if (bHit && HitResult.GetActor())
    {
        bIsWallSliding = true;

        // Z축 낙하 속도를 제한하여 천천히 미끄러지도록 처리
        FVector CurrentVelocity = GetCharacterMovement()->Velocity;
        if (CurrentVelocity.Z < WallSlideSpeed)
        {
            CurrentVelocity.Z = WallSlideSpeed;
            GetCharacterMovement()->Velocity = CurrentVelocity;
        }
    }
    else
    {
        bIsWallSliding = false;
    }
}
