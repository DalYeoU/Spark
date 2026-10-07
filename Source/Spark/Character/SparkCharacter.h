#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SparkCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class USparkComponent;
class USparkInteractionComponent;
class UAnimMontage;
struct FInputActionValue;

/**
 * ASparkCharacter
 * 
 * Spark 프로젝트의 메인 플레이어 캐릭터 클래스입니다.
 * 3D 퍼즐 플랫폼 환경에서 기본 이동, 시점 회전 및 카메라 조작을 담당합니다.
 */
UCLASS()
class SPARK_API ASparkCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // 기본 사양 및 컴포넌트 초기화
    ASparkCharacter();

    // 캐릭터 이동 및 입력 전달
    void Move(const FInputActionValue& Value);

    // 시점 회전 처리
    void Look(const FInputActionValue& Value);
    
    // 점프 실행 (Wall Slide 상태일 경우 Wall Jump로 분기)
    virtual void Jump() override;

    // 달리기 시작 입력 핸들러
    void StartSprint();

    // 달리기 종료 입력 핸들러
    void StopSprint();

    // 슬라이딩 시작 입력 핸들러 (키 누름)
    void StartSlide();

    // 슬라이딩 키 뗌 핸들러 (재입력 락 해제)
    void OnSlideKeyReleased();

    // 이동 입력 키를 뗐을 때 호출 (Move Input Action의 Completed에 연결)
    void OnMoveInputReleased();

    // 슬라이딩 종료 처리
    // bPlayEndMontage가 false면 낙하 등으로 슬라이드가 강제 중단된 경우로 보고 종료 몽타주 없이 즉시 State Machine에 제어를 넘긴다
    void StopSlide(bool bPlayEndMontage = true);

    // 상호작용 실행 입력 핸들러
    void Interact();

    // Run/Sprint 애니메이션의 발이 지면에 닿는 프레임(AnimNotify)에서 호출되어 스파크를 터뜨림
    void HandleFootstepNotify();

    // 카메라 스프링암 컴포넌트 안전한 게터
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    // 팔로우 카메라 컴포넌트 안전한 게터
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // 상호작용 컴포넌트 안전한 게터
    FORCEINLINE USparkInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

    FORCEINLINE UWidgetComponent* GetInteractionPromptWidgetComponent() const { return InteractionPromptWidgetComponent; }

    // 낙사 또는 HazardZone 오버랩시 엔진에서 호출되는 사망/실패 처리 오버라이드 함수
    virtual void FellOutOfWorld(const class UDamageType& DamageType) override;
    
    // 빠른 리스폰 처리 (세이브 시스템의 최신 체크포인트 위치로 이동)
    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void RespawnAtLastCheckpoint();

    // 레벨을 재시작하고 마지막 체크포인트에서 시작하도록 처리
    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void RestartLevelFromCheckpoint();

protected:
    // 캐릭터 초기화 및 게임플레이 시작 처리
    virtual void BeginPlay() override;

    // 프레임별 로직 업데이트
    virtual void Tick(float DeltaTime) override;

    // 바인딩된 입력 컴포넌트 설정
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    // 지면 착지 시 언리얼 엔진에서 자동으로 호출되는 오버라이드 함수
    virtual void Landed(const FHitResult& Hit) override;
    
    // 착지 이벤트 발생 시 세부 로직 및 피드백 처리 핸들러
    void HandleLanded(const FHitResult& Hit, float FallSpeed = 0.0f);

    // Crouch 시작/종료 시 캡슐이 움직인 만큼 카메라 오프셋에 반영해 다음 Tick부터 서서히 지워지도록 함
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    
    // 전방 벽면 감지 및 wall Slide 조건 검사
    void CheckWallSlide();

    // 쿨다운, 재사용 제한 등 Wall Slide 진입 가능 여부 판단
    bool CanEnterWallSlide() const;

    // 캐릭터 정면 라인 트레이스로 거의 수직인 벽면 감지
    bool TraceForWall(FHitResult& OutHitResult) const;

    // Wall Slide 중 낙하 속도를 WallSlideSpeed로 제한
    void ClampFallSpeedForWallSlide();

    // Wall Slide 중 주기적으로 마찰 Spark 연출을 갱신
    void UpdateWallSlideSpark(const FHitResult& HitResult);

    // Wall Jump 실행 로직 (LaunchCharacter 호출)
    void DoWallJump();
    
    // Wall Jump 이벤트 발생 시 세부 로직 및 피드백 처리 핸들러
    void HandleWallJump();

    // 상호작용 대상 변경 시 프롬프트 위젯을 새 대상에 재부착하거나 숨기는 핸들러
    UFUNCTION()
    void HandleInteractionTargetChanged(AActor* NewTarget);

    // 프롬프트가 항상 카메라를 향하도록 회전시키고 거리에 따라 크기를 보정
    void UpdateInteractionPromptTransform();

private:
    // 캐릭터와의 거리를 유지하고 벽 충돌 시 카메라를 당겨주는 스프링암 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    // 플레이어 뷰포트 시점을 제공하는 팔로우 카메라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;
    
    // 현재 벽 타기 상태 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsWallSliding = false;

    // Wall Jump 발생 직후 true로 세팅되고 착지 시 해제됨. ABP의 WallSlide->WallJump 전환 트리거로 사용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsWallJumping = false;

    // 벽 타기 시 낙하 속도 제한 (-150.0f = 천천히 미끄러짐)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallSlideSpeed = -150.0f;

    // 착지 시 낙하 속도(Z, 음수)가 이 값보다 작으면(3층 높이 정도) Light 착지로 판정. ABP에서 실시간 Velocity.Z 비교에 사용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float LightLandingFallSpeedThreshold = -1300.0f;

    // 착지 시 낙하 속도(Z, 음수)가 이 값보다 작으면(5층 높이 정도) Heavy 착지로 판정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float HeavyLandingFallSpeedThreshold = -1700.0f;

    // 착지 직전 속도(Z). 엔진이 착지 순간 Velocity를 0으로 초기화하므로, ABP는 라이브 Velocity 대신 이 캐시된 값을 읽어 Land 등급을 판정한다
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float LastLandingFallSpeed = 0.0f;

    // Light/Heavy 착지 시 그 즉시 이동 입력을 잠그는 시간. 이 시간이 지나기 전에 이동 입력이 들어오면 Move()에서 입력을 무시해 Land 애니메이션이 끝까지 재생된다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float LightLandingRecoveryDuration = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float HeavyLandingRecoveryDuration = 0.4f;

    // 이 시각(GetWorld()->GetTimeSeconds() 기준)이 되기 전까지는 착지 회복 중이라 이동 입력을 막는다
    float LandingRecoveryEndTime = -1.0f;

    // 캐릭터 정면 기준 벽 감지 트레이스 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallTraceDistance = 60.0f;

    // Wall Slide 애니메이션이 손을 뻗는 거리에 맞춰, 캡슐 표면에서 벽면까지 유지해야 하는 거리
    // 감지된 벽이 이보다 가까우면 캐릭터를 벽 노멀 방향으로 밀어내 애니메이션이 벽을 파고들지 않게 한다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallSlideDesiredSurfaceDistance = 25.0f;

    // 감지된 벽면의 노멀 벡터
    FVector CurrentWallNormal = FVector::ZeroVector;
    
    // Wall Jump시 벽 반대 방향으로 밀어내는 수평 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallJumpHorizontalImpulse = 500.0f;
    
    // Wall Jump시 위로 솟구치게 하는 수직 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallJumpVerticalImpulse = 500.0f;

    // Root Motion 전환 후 Velocity(=항상 캐릭터 정면 방향)로는 회전 기준을 못 잡으므로, 이번 프레임 입력 방향을 직접 저장해 Tick에서 그 방향으로 회전시킨다
    FVector DesiredFacingDirection = FVector::ZeroVector;

    // 이 각도(도) 이상 방향을 틀어야 하면 보간 없이 즉시 스냅 (반대 방향 전환 시 전진하며 도는 것 방지)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float LargeTurnSnapAngle = 100.0f;

    // Wall Jump 직후 같은 벽을 재감지하지 않도록 막는 쿨다운 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallJumpCooldownDuration = 0.4f;

    // 마지막 Wall Jump가 발생한 시각 (쿨다운 계산용)
    float LastWallJumpTime = -1.0f;

    // 착지 전까지 Wall Jump를 한 번만 허용하기 위한 상태 플래그
    bool bHasWallJumpedSinceGrounded = false;
    
    // 리스폰에 활용할 시작/체크포인트 위치
    FVector RespawnLocation = FVector::ZeroVector;

    // 리스폰 직후 화면이 검은색에서 밝아지는 데 걸리는 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float RespawnFadeInDuration = 2.0f;

    // 실패 감지 직후 화면이 검은색으로 어두워지는 데 걸리는 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float FailureFadeOutDuration = 0.4f;

    // 실패 시 노출할 안내 문구
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FText FailureMessage = FText::FromString(TEXT("UNIT OFFLINE"));

    // 완전 암전 후 안내 문구(및 체크포인트 복구 연출)를 노출하는 연출용 대기 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float FailureMessageDuration = 1.2f;

    // Fade Out 종료 후 안내 문구를 노출하기 위한 타이머
    FTimerHandle FailureFadeOutTimerHandle;

    // 안내 문구 노출 종료 후 텔레포트를 실행하기 위한 타이머
    FTimerHandle FailureMessageTimerHandle;

    // Fade In 종료 후 입력을 복구하기 위한 타이머
    FTimerHandle FailureFadeInTimerHandle;
    
    // Spark 연출 및 라이트 생성을 담당하는 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spark", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USparkComponent> SparkComponent;

    // 상호작용 대상 탐지 및 상호작용 실행을 담당하는 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USparkInteractionComponent> InteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

    // 프롬프트 위젯이 현재 실제로 부착되어 있는 대상 (가려짐 트레이스에서 자신을 무시하기 위한 용도)
    TWeakObjectPtr<AActor> InteractionPromptAttachedActor;

    // 포인트 마커를 대상 표면 위로 띄우는 높이 (라인은 여기서부터 위로 그려짐)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    float InteractionPromptHeightOffset = 0.0f;

    // 거리에 따른 프롬프트 크기 보정 기준 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    float InteractionPromptReferenceDistance = 250.0f;

    // DrawSize는 크게 유지한 채 실제 표시 크기만 축소하기 위한 배율
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    float InteractionPromptBaseScale = 0.25f;
    
    // Wall Slide Spark 방출 타이밍 조절용 변수
    float LastWallSlideSparkTime = 0.0f;

    // 슬라이딩/달리기 상태 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    bool bIsSliding = false;

    // 슬라이드 키를 꾹 누르고 있을 때 반복 실행을 방지하기 위한 키 입력 플래그
    bool bSlideKeyHeld = false;

    // 슬라이딩 중에는 Move()가 이동 입력을 무시하므로, 슬라이드 종료 시 Idle/Run 여부 판단을 위해 최근 입력값을 따로 저장한다
    FVector2D LastMovementInput = FVector2D::ZeroVector;

    // 달리기 및 슬라이딩 속도 설정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
    float WalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
    float SprintSpeed = 950.0f;

    // 슬라이드 속도: SprintSpeed(950)의 약 1.3배로 설정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    float SlideImpulse = 1235.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    float MinSlideEntrySpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    float SlideDuration = 0.7f;

    // 슬라이딩 캡슐 절반 높이 (기본 88.0f -> 슬라이딩 시 44.0f)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    float SlideCapsuleHalfHeight = 44.0f;

    float DefaultCapsuleHalfHeight = 88.0f;
    float DefaultGroundFriction = 8.0f;
    float DefaultBrakingDeceleration = 2048.0f;

    // 슬라이드 시작 시 결정된 수평 속도. 매 프레임 다시 강제 적용해 도중에 Velocity가 틀어져도 복구되게 한다
    FVector SlideVelocity = FVector::ZeroVector;

    // 마찰 스파크 타이밍 조절용 변수
    float LastSlideSparkTime = 0.0f;

    FTimerHandle SlideTimerHandle;
    float SlideElapsedTime = 0.0f;

    // Crouch로 인한 캡슐 움직임을 상쇄했다가 서서히 0으로 되돌리는 카메라 보정 오프셋
    float CrouchEyeOffsetZ = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    float CrouchEyeOffsetInterpSpeed = 12.0f;

    // 슬라이드 시작 시 재생하는 Montage (달리기->슬라이드 전환 포함)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> SlideStartMontage;

    // SlideStartMontage에서 실제 슬라이드 포즈가 시작되는 지점의 Montage Section 이름
    // 비워두면 처음부터 재생한다. 달리는 부분을 건너뛰려면 Montage 에디터에서 Section을 나눠 이름을 지정한다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    FName SlideStartMontageSection = NAME_None;

    // 슬라이드 종료 후 Run으로 이어지는 Montage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> SlideEndToRunMontage;

    // 슬라이드 종료 후 Sprint로 이어지는 Montage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> SlideEndToSprintMontage;

    // 슬라이드 종료 후 이동 입력이 없을 때 이어지는 Montage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> SlideEndToIdleMontage;

    // 재생 중인 슬라이드 종료 Montage. 재생 도중 이동 입력이 풀리거나(Run/Sprint) 낙하가 시작되면 Tick에서 직접 끊는다
    TObjectPtr<UAnimMontage> ActiveSlideEndMontage;

    // true면 이동 입력 해제로도 끊는다(Run/Sprint 몽타주). Idle 몽타주는 낙하 감지로만 끊는다
    bool bActiveSlideEndMontageCutsOnInputRelease = false;

    // 슬라이딩 가능 여부 검사
    bool CanSlide() const;

    // 지면 마찰 스파크 갱신
    void UpdateGroundSparks(float DeltaTime);

    // 착지 충돌 결과에서 실제 밟고있는 머티리얼 정보를 반환
    FHitResult ResolveLandingHit(const FHitResult& InHit) const;

    // Fade Out이 끝난 뒤(완전 암전 상태) 안내 문구를 노출하고 연출용 대기를 시작
    void ShowFailureMessageAndWait();

    // 안내 문구 연출이 끝난 뒤 체크포인트로 텔레포트하고 Fade In 및 입력 복구를 진행
    void TeleportToCheckpointAndFadeIn();

};
