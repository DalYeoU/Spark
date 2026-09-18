#include "SparkCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "InputActionValue.h"

#include "Components/SparkComponent.h"
#include "Components/SparkInteractionComponent.h"
#include "UI/SparkInteractionPromptWidget.h"
#include "Blueprint/UserWidget.h"
#include "Save/SparkSaveSubsystem.h"
#include "Save/SparkSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/SparkPlayerController.h"
#include "UI/SparkFailureMessageWidget.h"

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
    GetCharacterMovement()->AirControl = 0.65f;         // 공중 제어력
    GetCharacterMovement()->GravityScale = 1.2f;        // 중력 스케일
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;      // 기본 걷기 최대 속도
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

    // InteractionComponent 생성
    InteractionComponent = CreateDefaultSubobject<USparkInteractionComponent>(TEXT("InteractionComponent"));

    // 상호작용 대상에 재부착되는 월드 스페이스 프롬프트 위젯, 초기에는 대상이 없으므로 숨김
    InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidgetComponent"));
    InteractionPromptWidgetComponent->SetupAttachment(RootComponent);
    // 매 틱 월드 트랜스폼을 직접 계산해서 넣으므로, 캐릭터 회전(이동 시 계속 회전)이 그대로 합성되지 않도록 부모와 독립시킴
    InteractionPromptWidgetComponent->SetUsingAbsoluteLocation(true);
    InteractionPromptWidgetComponent->SetUsingAbsoluteRotation(true);
    InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    InteractionPromptWidgetComponent->SetDrawSize(FVector2D(800.0f, 220.0f));
    InteractionPromptWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
    InteractionPromptWidgetComponent->SetVisibility(false);
    InteractionPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionPromptWidgetComponent->SetTwoSided(true);
    // 기본 Blend Mode는 알파를 이진 처리해 텍스트 가장자리가 계단현상으로 깨지므로 Transparent로 전환
    InteractionPromptWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
    // 라이트를 받아 바닥에 그림자가 지는 것을 방지
    InteractionPromptWidgetComponent->SetCastShadow(false);
}

void ASparkCharacter::Interact()
{
    if (InteractionComponent)
    {
        InteractionComponent->PrimaryInteract();
    }
}

void ASparkCharacter::HandleInteractionTargetChanged(AActor* NewTarget)
{
    if (!InteractionPromptWidgetComponent) return;

    if (NewTarget)
    {
        // 캐릭터에 계속 붙여둔 채 매 틱 월드 좌표만 갱신
        InteractionPromptWidgetComponent->SetVisibility(true);
        InteractionPromptWidgetComponent->SetHiddenInGame(false);
        InteractionPromptAttachedActor = NewTarget;

        // 보이자마자 즉시 위치를 갱신해 다음 Tick까지의 한 프레임 동안 이전 위치가 잠깐 보이는 것을 방지
        UpdateInteractionPromptTransform();
    }
    else
    {
        InteractionPromptWidgetComponent->SetVisibility(false);
        InteractionPromptAttachedActor = nullptr;
    }
}

void ASparkCharacter::UpdateInteractionPromptTransform()
{
    AActor* TargetActor = InteractionPromptAttachedActor.Get();
    if (!InteractionPromptWidgetComponent || !InteractionPromptWidgetComponent->IsVisible() || !FollowCamera || !TargetActor) return;

    // X/Y는 액터 위치를 그대로 쓰고 Z만 바운딩 박스 상단 높이로 계산 (바운딩 박스 중심 X/Y는 비대칭 컴포넌트 때문에 실제 위치와 어긋날 수 있음)
    FVector Origin, BoxExtent;
    TargetActor->GetActorBounds(false, Origin, BoxExtent);
    const FVector ActorLocation = TargetActor->GetActorLocation();
    const FVector PromptLocation = FVector(ActorLocation.X, ActorLocation.Y, Origin.Z + BoxExtent.Z + InteractionPromptHeightOffset);
    InteractionPromptWidgetComponent->SetWorldLocation(PromptLocation);

    const FVector CameraLocation = FollowCamera->GetComponentLocation();

    // 카메라의 Up 벡터까지 그대로 따라가는 빌보드
    const FRotator CameraRotation = FollowCamera->GetComponentRotation();
    const FRotator TargetRotation(-CameraRotation.Pitch, CameraRotation.Yaw + 180.0f, CameraRotation.Roll);
    const FRotator CurrentRotation = InteractionPromptWidgetComponent->GetComponentRotation();
    InteractionPromptWidgetComponent->SetWorldRotation(FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 15.0f));

    // 거리 비례로 크기를 보정해 멀어져도 가독성 유지
    const float Distance = FVector::Dist(CameraLocation, PromptLocation);
    const float Scale = InteractionPromptBaseScale * FMath::Clamp(Distance / InteractionPromptReferenceDistance, 0.4f, 1.0f);
    InteractionPromptWidgetComponent->SetWorldScale3D(FVector(Scale));

    // 카메라와 프롬프트 사이에 장애물이 있으면 가려짐 처리
    FHitResult OcclusionHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    // 프롬프트가 실제로 붙어있는 대상 자신을 장애물로 오판하지 않도록 트레이스에서 제외
    QueryParams.AddIgnoredActor(InteractionPromptAttachedActor.Get());
    const bool bOccluded = GetWorld()->LineTraceSingleByChannel(OcclusionHit, CameraLocation, PromptLocation, ECC_Visibility, QueryParams);
    InteractionPromptWidgetComponent->SetHiddenInGame(bOccluded);
}

void ASparkCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 시작 시점의 위치를 초기 리스폰 기본값으로 기억
    RespawnLocation = GetActorLocation();

    // 기본 캡슐 및 마찰 설정 캐싱
    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        DefaultCapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
    }
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        DefaultGroundFriction = 8.0f;
        DefaultBrakingDeceleration = 2048.0f;
        MoveComp->GroundFriction = DefaultGroundFriction;
        MoveComp->BrakingDecelerationWalking = DefaultBrakingDeceleration;
        MoveComp->MaxWalkSpeed = WalkSpeed;

        // 슬라이드는 엔진 Crouch 시스템을 재사용해 캡슐 축소/위치 보정을 안전하게 처리
        MoveComp->NavAgentProps.bCanCrouch = true;
        MoveComp->SetCrouchedHalfHeight(SlideCapsuleHalfHeight);
    }

    // 상호작용 대상 변경에 따라 프롬프트 위젯을 재부착
    if (InteractionComponent)
    {
        InteractionComponent->OnInteractionTargetChanged.AddDynamic(this, &ASparkCharacter::HandleInteractionTargetChanged);
    }

    if (InteractionPromptWidgetComponent)
    {
        UUserWidget* RawWidget = InteractionPromptWidgetComponent->GetWidget();
        if (USparkInteractionPromptWidget* PromptWidget = Cast<USparkInteractionPromptWidget>(RawWidget))
        {
            PromptWidget->BindInteractionComponent(InteractionComponent);
        }
    }

    // 체크포인트 복원 플래그가 켜져 있을 때만 위치 복원 (레벨 재시작 또는 이어하기 시)
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance) return;

    USparkSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USparkSaveSubsystem>();
    if (!SaveSubsystem || !SaveSubsystem->ShouldRestoreFromCheckpoint()) return;

    // 복원 1회 처리 후 플래그 초기화 (다음 일반 시작 시 영향 방지)
    SaveSubsystem->SetShouldRestoreFromCheckpoint(false);

    USparkSaveGame* SaveData = SaveSubsystem->GetCurrentSaveData();
    if (!SaveData)
    {
        SaveData = SaveSubsystem->LoadGameData();
    }
    if (!SaveData || SaveData->CheckpointId.IsNone()) return;

    const FName CurrentLevelName = *UGameplayStatics::GetCurrentLevelName(this, true);
    if (SaveData->LevelName != CurrentLevelName) return;

    const FTransform& SavedTransform = SaveData->PlayerTransform;
    TeleportTo(SavedTransform.GetLocation(), SavedTransform.Rotator());
    RespawnLocation = SavedTransform.GetLocation();
}

void ASparkCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 매 프레임 Wall Slide 여부 감지
    CheckWallSlide();

    // 지면 마찰 스파크 및 슬라이드 감속 감지
    UpdateGroundSparks(DeltaTime);

    // 상호작용 프롬프트가 표시 중이면 카메라를 향하도록 회전/거리 보정
    UpdateInteractionPromptTransform();

    // Crouch 보정 오프셋을 서서히 0으로 되돌려 카메라가 부드럽게 이동하도록 함
    if (!FMath::IsNearlyZero(CrouchEyeOffsetZ) && CameraBoom)
    {
        CrouchEyeOffsetZ = FMath::FInterpTo(CrouchEyeOffsetZ, 0.0f, DeltaTime, CrouchEyeOffsetInterpSpeed);
        CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, CrouchEyeOffsetZ));
    }
}

void ASparkCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // 캡슐이 아래로 내려간 만큼 카메라를 위로 보정해 시야가 즉시 튀지 않게 함
    CrouchEyeOffsetZ += ScaledHalfHeightAdjust;
}

void ASparkCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // 캡슐이 위로 올라간 만큼 카메라를 아래로 보정해 시야가 즉시 튀지 않게 함
    CrouchEyeOffsetZ -= ScaledHalfHeightAdjust;
}

void ASparkCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASparkCharacter::Move(const FInputActionValue& Value)
{
    // 슬라이딩 중에는 플레이어의 이동 입력을 무시하여 무한 미끄러짐 방지
    if (bIsSliding) return;

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

    // 슬라이딩 도중 점프 시 슬라이딩을 종료하고 도약
    if (bIsSliding)
    {
        StopSlide();
    }

    // 달리기 중 점프 시 수평 속도가 너무 과하게 튀어나가지 않도록 자연스럽게 완충
    if (bIsSprinting && GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
    {
        FVector HorizontalVelocity = GetVelocity();
        HorizontalVelocity.Z = 0.0f;
        const float CurrentSpeed = HorizontalVelocity.Size();
        const float MaxSprintJumpHorizontalSpeed = 780.0f;

        if (CurrentSpeed > MaxSprintJumpHorizontalSpeed)
        {
            FVector ClampedVelocity = HorizontalVelocity.GetSafeNormal() * MaxSprintJumpHorizontalSpeed;
            ClampedVelocity.Z = GetCharacterMovement()->Velocity.Z;
            GetCharacterMovement()->Velocity = ClampedVelocity;
        }
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
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    // 실패 연출 중 조작이 끼어들지 않도록 입력 차단
    if (PlayerController)
    {
        DisableInput(PlayerController);
    }

    // 사망 직전 마지막 스파크로 주변을 잠깐 밝혔다가 Fade Out과 함께 꺼지도록 연출
    if (SparkComponent)
    {
        SparkComponent->SpawnSparkLight(GetActorLocation(), 500000.0f, 100000.0f, FailureFadeOutDuration);
    }

    // 화면을 검은색으로 가려 이후 텔레포트 순간 이동을 감춤
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FailureFadeOutDuration, FLinearColor::Black, false, true);
    }

    // 완전 암전된 뒤에 안내 문구를 노출하기 위해 Fade Out이 끝날 때까지 대기
    GetWorldTimerManager().SetTimer(FailureFadeOutTimerHandle, this, &ASparkCharacter::ShowFailureMessageAndWait, FailureFadeOutDuration, false);
}

void ASparkCharacter::ShowFailureMessageAndWait()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    // 실패 안내 문구 노출 (SYSTEM / UNIT OFFLINE + 체크포인트 복구 연출은 WBP_FailureMessage 애니메이션에서 처리)
    if (ASparkPlayerController* SparkPlayerController = Cast<ASparkPlayerController>(PlayerController))
    {
        if (USparkFailureMessageWidget* FailureWidget = SparkPlayerController->GetFailureMessageWidget())
        {
            FailureWidget->ShowMessage(FailureMessage);
        }
    }

    GetWorldTimerManager().SetTimer(FailureMessageTimerHandle, this, &ASparkCharacter::TeleportToCheckpointAndFadeIn, FailureMessageDuration, false);
}

void ASparkCharacter::TeleportToCheckpointAndFadeIn()
{
    // 속도 및 움직임 초기화 후 리스폰 위치로 이동
    GetCharacterMovement()->StopActiveMovement();
    GetCharacterMovement()->Velocity = FVector::ZeroVector;

    // 세이브 서브시스템에서 마지막 체크포인트 위치 조회
    USparkSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<USparkSaveSubsystem>() : nullptr;
    USparkSaveGame* SaveData = SaveSubsystem ? SaveSubsystem->GetCurrentSaveData() : nullptr;
    if (SaveSubsystem && !SaveData)
    {
        SaveData = SaveSubsystem->LoadGameData();
    }

    // 체크포인트가 있으면 복원, 없으면 초기 위치로 이동
    if (SaveData && !SaveData->CheckpointId.IsNone())
    {
        TeleportTo(SaveData->PlayerTransform.GetLocation(), SaveData->PlayerTransform.Rotator());
    }
    else
    {
        SetActorLocation(RespawnLocation);
    }

    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    // 텔레포트 자체는 즉시 처리하고, 화면만 검은색에서 서서히 밝아지게 해서 순간 이동의 위화감을 가림
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, RespawnFadeInDuration, FLinearColor::Black, false, true);
    }

    // 실패 안내 문구 숨김
    if (ASparkPlayerController* SparkPlayerController = Cast<ASparkPlayerController>(PlayerController))
    {
        if (USparkFailureMessageWidget* FailureWidget = SparkPlayerController->GetFailureMessageWidget())
        {
            FailureWidget->HideMessage();
        }
    }

    // Fade In이 끝난 뒤 입력 복구
    if (PlayerController)
    {
        FTimerDelegate EnableInputDelegate = FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
        {
            EnableInput(PlayerController);
        });
        GetWorldTimerManager().SetTimer(FailureFadeInTimerHandle, EnableInputDelegate, RespawnFadeInDuration, false);
    }
}

void ASparkCharacter::RestartLevelFromCheckpoint()
{
    // 레벨 재시작 후 BeginPlay에서 체크포인트 위치로 복원되도록 플래그 활성화
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        if (USparkSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USparkSaveSubsystem>())
        {
            SaveSubsystem->SetShouldRestoreFromCheckpoint(true);
        }
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->RestartLevel();
    }
}

void ASparkCharacter::StartSprint()
{
    bIsSprinting = true;
    if (!bIsSliding && GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
#if WITH_EDITOR
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1001, 1.5f, FColor::Yellow, FString::Printf(TEXT("[Sprint ON] MaxSpeed: %.0f"), SprintSpeed));
    }
#endif
}

void ASparkCharacter::StopSprint()
{
    bIsSprinting = false;
    if (!bIsSliding && GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
#if WITH_EDITOR
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1001, 1.5f, FColor::Silver, FString::Printf(TEXT("[Sprint OFF] MaxSpeed: %.0f"), WalkSpeed));
    }
#endif
}

bool ASparkCharacter::CanSlide() const
{
    // 이미 슬라이딩 중이거나 키를 꾹 누르고 있는 상태(손을 떼지 않음)면 재발동 절대 불가
    if (bIsSliding || bSlideKeyHeld) return false;
    if (!GetCharacterMovement() || !GetCharacterMovement()->IsMovingOnGround()) return false;
    if (!bIsSprinting) return false;

    const float CurrentSpeed = GetVelocity().Size2D();
    if (CurrentSpeed < MinSlideEntrySpeed) return false;

    return true;
}

void ASparkCharacter::OnSlideKeyReleased()
{
    // 손가락으로 키를 뗐을 때만 다음 슬라이드 입력이 가능하도록 락 해제
    bSlideKeyHeld = false;
}

void ASparkCharacter::StartSlide()
{
    if (!CanSlide()) return;

    bSlideKeyHeld = true; // 키를 누른 즉시 락을 걸어 손을 뗄 때까지 1회 탭으로 고정
    bIsSliding = true;
    SlideElapsedTime = 0.0f; // 슬라이드 경과 시간 초기화

    // Crouch()가 캡슐 축소와 위치 보정(Sweep 포함)을 함께 처리
    Crouch();

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 슬라이드 중에는 마찰과 브레이킹을 0으로 설정하여 감속 슬라이딩 구현
        MoveComp->BrakingDecelerationWalking = 0.0f;
        MoveComp->GroundFriction = 0.0f;
        MoveComp->MaxWalkSpeed = SlideImpulse;

        // 전방 슬라이드 방향 계산
        FVector SlideDirection = GetVelocity().GetSafeNormal2D();
        if (SlideDirection.IsNearlyZero())
        {
            SlideDirection = GetActorForwardVector();
        }

        // 지면을 따라 전방으로 초고속 슬라이드 추진력 주입
        MoveComp->Velocity = SlideDirection * SlideImpulse;
    }

    // 즉시 첫 마찰 스파크 방출
    if (SparkComponent && GetCharacterMovement())
    {
        const FHitResult FloorHit = ResolveLandingHit(GetCharacterMovement()->CurrentFloor.HitResult);
        SparkComponent->TriggerSlideSpark(FloorHit);
        LastSlideSparkTime = GetWorld()->GetTimeSeconds();
    }

#if WITH_EDITOR
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1002, 1.5f, FColor::Cyan, TEXT("[Slide STARTED - Sliding Forward!]"));
    }
#endif
}

void ASparkCharacter::StopSlide()
{
    if (!bIsSliding) return;

    bIsSliding = false;
    SlideElapsedTime = 0.0f;

    // UnCrouch()가 캡슐 복구와 위치 보정(Sweep 포함)을 함께 처리
    UnCrouch();

    // 마찰력 및 최대 이동속도 즉시 복구
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->GroundFriction = DefaultGroundFriction;
        MoveComp->BrakingDecelerationWalking = DefaultBrakingDeceleration;
        MoveComp->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
    }

#if WITH_EDITOR
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1002, 1.5f, FColor::Green, TEXT("[Slide ENDED - Normal Restored]"));
    }
#endif
}

void ASparkCharacter::UpdateGroundSparks(float DeltaTime)
{
    // 슬라이딩 중일 때는 정해진 시간(0.8초) 동안 시원하게 미끄러진 뒤 종료
    if (bIsSliding)
    {
        SlideElapsedTime += DeltaTime;
        const float CurrentTime = GetWorld()->GetTimeSeconds();

        // 0.8초 경과 시 슬라이딩 종료
        if (SlideElapsedTime >= SlideDuration)
        {
            StopSlide();
            return;
        }

        // 지면에 닿아 있는 동안 주기적 마찰 스파크 방출 (0.12초 주기)
        if (SparkComponent && GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
        {
            if (CurrentTime - LastSlideSparkTime >= 0.12f)
            {
                LastSlideSparkTime = CurrentTime;
                const FHitResult FloorHit = ResolveLandingHit(GetCharacterMovement()->CurrentFloor.HitResult);
                SparkComponent->TriggerSlideSpark(FloorHit);
            }
        }
        return;
    }

    // 달리기 중일 때 발바닥 스파크 (지면 상태이고 빠르게 달릴 때)
    if (!SparkComponent || !GetCharacterMovement() || !GetCharacterMovement()->IsMovingOnGround()) return;

    const float Speed = GetVelocity().Size2D();
    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if (bIsSprinting && Speed > WalkSpeed + 50.0f)
    {
        if (CurrentTime - LastSprintSparkTime >= 0.28f)
        {
            LastSprintSparkTime = CurrentTime;
            const FHitResult FloorHit = ResolveLandingHit(GetCharacterMovement()->CurrentFloor.HitResult);
            SparkComponent->TriggerSprintSpark(FloorHit);
        }
    }
}

