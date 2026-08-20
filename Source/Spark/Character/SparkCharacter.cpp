#include "SparkCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "InputActionValue.h"

#include "Components/SparkComponent.h"

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
    
    // SparkComponent 생성
    SparkComponent = CreateDefaultSubobject<USparkComponent>(TEXT("SparkComponent"));
}

void ASparkCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 시작 시점의 위치를 초기 리스폰 위치로 기억
    RespawnLocation = GetActorLocation();
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
    // Super::Landed(Hit) 호출 직전에 속도를 구함
    const float RawFallSpeed = GetCharacterMovement() ? GetCharacterMovement()->Velocity.Z : 0.0f;
    
    Super::Landed(Hit);
    
    // 착지 이벤트 핸들러 호출
    HandleLanded(Hit, RawFallSpeed);
}

void ASparkCharacter::HandleLanded(const FHitResult& Hit, float FallSpeed)
{
    // 착지하면 Wall Jump 쿨다운과 재사용 제한을 초기화해서 새 벽에 바로 붙을 수 있게 함
    LastWallJumpTime = -1.0f;
    bHasWallJumpedSinceGrounded = false;
    
    // 바닥 정보 획득 및 Landing Spark 트리거
    if (SparkComponent)
    {
        const FHitResult LandingHit = ResolveLandingHit(Hit);
        SparkComponent->TriggerLandingSpark(LandingHit, FallSpeed);
    }
}

FHitResult ASparkCharacter::ResolveLandingHit(const FHitResult& InHit) const
{
    FHitResult LandingHit = InHit;
    float Radius = 34.0f;
    float HalfHeight = 88.0f;
    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        Radius = CapsuleComp->GetScaledCapsuleRadius();
        HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
    }
    
    // 무브먼트 컴포넌트가 찾은 실제 바닥 정보 사용
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (MoveComp->CurrentFloor.IsWalkableFloor() && MoveComp->CurrentFloor.HitResult.IsValidBlockingHit())
        {
            LandingHit = MoveComp->CurrentFloor.HitResult;
        }
    }
    
    // 머티리얼 정보가 없으면 캡슐 스윕으로 보정함
    if (!LandingHit.PhysMaterial.IsValid())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex = true;
        QueryParams.bReturnPhysicalMaterial = true;

        const FVector Start = GetActorLocation();
        const FVector End = Start - FVector(0.0f, 0.0f, 20.0f);
        const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
            
        FHitResult SweepHit;
        if (GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, ECC_Visibility, CapsuleShape, QueryParams))
        {
            LandingHit.PhysMaterial = SweepHit.PhysMaterial;
            LandingHit.ImpactPoint = SweepHit.ImpactPoint;
            LandingHit.ImpactNormal = SweepHit.ImpactNormal;
        }
    }
    
    // 만약 여전히 ImpactPoint가 비어있다면 발바닥 위치로 최종 보정
    if (LandingHit.ImpactPoint.IsNearlyZero())
    {
        LandingHit.ImpactPoint = GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight);
    }
    return LandingHit;
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
    
    // 정면 벽 감지
    FHitResult HitResult;
    if (TraceForWall(HitResult))
    {
        bIsWallSliding = true;
        CurrentWallNormal = HitResult.ImpactNormal;
        
        // 낙하 속도 감속
        ClampFallSpeedForWallSlide();
        
        // 마찰 스파크 연출 갱신
        UpdateWallSlideSpark(HitResult);
    }
    else
    {
        bIsWallSliding = false;
        CurrentWallNormal = FVector::ZeroVector;
    }
}

void ASparkCharacter::UpdateWallSlideSpark(const FHitResult& HitResult)
{
    if (!SparkComponent) return;
    
    // 0.15초 주기 검사
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastWallSlideSparkTime <= 0.15f)
    {
        return;
    }
    LastWallSlideSparkTime = CurrentTime;
    
    // 발 밑 높이로 Spark 위치 보정
    float CapsuleHalfHeight = 88.0f;
    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
    }
    
    FHitResult SlideHit = HitResult;
    SlideHit.ImpactPoint.Z = GetActorLocation().Z - CapsuleHalfHeight;
    
    SparkComponent->TriggerWallSlideSpark(SlideHit);
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
    QueryParams.bReturnPhysicalMaterial = true;
    QueryParams.bTraceComplex = true;

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
    // 벽을 차고 나가는 그 위치와 법선으로 강한 Wall Jump Spark 트리거
    if (SparkComponent)
    {
        // 정면 벽 트레이스 지점 또는 벽 노멀 반대 방향 접촉 위치
        FHitResult HitResult;
        if (TraceForWall(HitResult))
        {
            SparkComponent->TriggerWallJumpSpark(HitResult);
        }
        else
        {
            // 혹시 트레이스 직후 미세하게 떨어졌다면 현재 위치와 노멀로 대체한 HitResult 구성
            FHitResult FallbackHit;
            FallbackHit.ImpactPoint = GetActorLocation();
            FallbackHit.ImpactNormal = CurrentWallNormal;
            FallbackHit.Location = GetActorLocation();
            SparkComponent->TriggerWallJumpSpark(FallbackHit);
        }
    }
}

void ASparkCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
    RespawnAtLastCheckpoint();
}

void ASparkCharacter::RespawnAtLastCheckpoint()
{
    // 속도 및 움직임 초기화 후 리스폰 위치로 이동
    GetCharacterMovement()->StopActiveMovement();
    GetCharacterMovement()->Velocity = FVector::ZeroVector;

    SetActorLocation(RespawnLocation);

    // 텔레포트 자체는 즉시 처리하고, 화면만 검은색에서 서서히 밝아지게 해서 순간 이동의 위화감을 가림
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
        {
            CameraManager->StartCameraFade(1.0f, 0.0f, RespawnFadeInDuration, FLinearColor::Black, false, true);
        }
    }
}
