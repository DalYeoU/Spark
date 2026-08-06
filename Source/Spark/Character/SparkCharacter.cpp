#include "SparkCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

ASparkCharacter::ASparkCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 마우스로 카메라 시점을 돌려도 캐릭터 몸통은 따라 돌지 않도록 비활성화
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 이동 입력 방향으로 몸통이 자연스럽게 돌아가도록 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    // 입력에 즉각 반응하도록 기본 이동 스펙 설정
    GetCharacterMovement()->JumpZVelocity = 450.0f;     // 점프 높이 조절
    GetCharacterMovement()->AirControl = 0.85f;         // 공중 제어력 (지상 대비 약 85% 반응성)
    GetCharacterMovement()->GravityScale = 1.2f;        // 중력 스케일
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;      // 최대 이동 속도
    GetCharacterMovement()->MaxAcceleration = 4096.0f;  // 입력 즉시 최대 속도에 도달하도록 가속도 상향

    // 3인칭 팔로우 시점을 위한 스프링암 생성, 마우스 회전과 동기화
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // 스프링암 끝에 달아 실제 화면을 그리는 카메라 컴포넌트 생성
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
    
    // 매 프레임 Wall Slide 여부 감지
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
        // 카메라를 위아래로 기울여도 수직 이동이 섞이지 않도록 Yaw 값만 추출
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

        // 시점 기준 전방(X)과 우측(Y) 방향 벡터 계산
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 계산한 방향으로 이동 입력 전달
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASparkCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 좌우/상하 시점 회전 입력 반영
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
    // 착지 확인용 임시 로그 (추후 SparkComponent 연동 지점)
    FString SurfaceName = Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT("Unknown Surface");
    UE_LOG(LogTemp, Log, TEXT("SparkCharacter Landed on: %s"), *SurfaceName);

    // 착지하면 Wall Jump 쿨다운과 재사용 제한을 초기화해서 새 벽에 바로 붙을 수 있게 함
    LastWallJumpTime = -1.0f;
    bHasWallJumpedSinceGrounded = false;
}

// 공중에서 정면 벽을 감지해 Wall Slide 상태를 갱신하고, 슬라이드 중이면 낙하 속도를 늦춘다.
void ASparkCharacter::CheckWallSlide()
{
    // 땅에 서 있거나(공중이 아니거나), 쿨다운/재사용 제한에 걸려 있으면 애초에 벽을 감지할 필요가 없으므로 트레이스 없이 바로 상태를 해제
    if (!GetCharacterMovement()->IsFalling() || !CanEnterWallSlide())
    {
        bIsWallSliding = false;
        CurrentWallNormal = FVector::ZeroVector;
        return;
    }

    FHitResult HitResult;
    if (TraceForWall(HitResult))
    {
        // 벽을 감지하면 슬라이드 상태로 전환하고, 다음 벽점프 계산에 쓸 노멀을 저장
        bIsWallSliding = true;
        CurrentWallNormal = HitResult.ImpactNormal;

        // 슬라이드 중에는 낙하 속도를 늦춰서 벽에 붙어 미끄러지는 느낌을 준다
        ClampFallSpeedForWallSlide();
    }
    else
    {
        // 벽이 없으면 일반 낙하 상태이므로 슬라이드 상태와 저장된 노멀을 모두 초기화
        bIsWallSliding = false;
        CurrentWallNormal = FVector::ZeroVector;
    }
}

bool ASparkCharacter::CanEnterWallSlide() const
{
    // 착지 전에 Wall Jump를 이미 썼다면 재진입 금지
    if (bHasWallJumpedSinceGrounded)
    {
        return false;
    }

    // 쿨다운 중이면 같은 벽을 다시 감지하지 않게 막아 연속 Wall Jump 방지
    if (GetWorld()->GetTimeSeconds() - LastWallJumpTime < WallJumpCooldownDuration)
    {
        return false;
    }

    return true;
}

bool ASparkCharacter::TraceForWall(FHitResult& OutHitResult) const
{
    // 정면으로 라인 트레이스를 쏴서 벽면 감지
    const FVector Start = GetActorLocation();
    const FVector End = Start + (GetActorForwardVector() * WallTraceDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // Wall Slide 전용 Trace Channel 사용 (Project Settings에서 이름을 "Wall"로 지정해둬야 함)
    const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_GameTraceChannel1, QueryParams);

    // 거의 수직인 면만 벽으로 인정해 완만한 경사면은 걸러냄
    return bHit && OutHitResult.GetActor() && FMath::Abs(OutHitResult.ImpactNormal.Z) < 0.3f;
}

void ASparkCharacter::ClampFallSpeedForWallSlide()
{
    // 낙하 속도(Z)를 제한해 천천히 미끄러지게 함
    FVector CurrentVelocity = GetCharacterMovement()->Velocity;
    if (CurrentVelocity.Z < WallSlideSpeed)
    {
        CurrentVelocity.Z = WallSlideSpeed;
        GetCharacterMovement()->Velocity = CurrentVelocity;
    }
}

void ASparkCharacter::Jump()
{
    // 벽을 타고 있는 상태라면 일반 점프 대신 Wall Jump 실행
    if (bIsWallSliding)
    {
        DoWallJump();
        
        return;
    }
    Super::Jump();
}

void ASparkCharacter::DoWallJump()
{
    // 벽 반대 방향과 위쪽 힘을 합쳐 튕겨나가는 점프 벡터 계산
    FVector JumpDirection = (CurrentWallNormal * WallJumpHorizontalImpulse) + (FVector::UpVector * WallJumpVerticalImpulse);

    // 캐릭터가 벽 반대쪽을 보도록 회전
    FRotator TargetRotation = CurrentWallNormal.Rotation();
    SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));

    // 계산한 방향으로 캐릭터를 튕겨냄
    LaunchCharacter(JumpDirection, true, true);

    // Wall Slide 상태 해제 및 쿨다운 시작 시각 기록, 착지 전까지 재사용 금지 처리
    bIsWallSliding = false;
    LastWallJumpTime = GetWorld()->GetTimeSeconds();
    bHasWallJumpedSinceGrounded = true;
    
    // 이벤트 핸들러 호출
    HandleWallJump();
}

void ASparkCharacter::HandleWallJump()
{
    // Wall Jump 발생 로그 (추후 SparkComponent 피드백 연동 지점)
    UE_LOG(LogTemp, Log, TEXT("SparkCharacter Executed Wall Jump!"));   
}
