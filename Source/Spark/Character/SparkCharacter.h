#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SparkCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USparkComponent;
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

    // 카메라 스프링암 컴포넌트 안전한 게터
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    // 팔로우 카메라 컴포넌트 안전한 게터
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    
    // 낙사 또는 HazardZone 오버랩시 엔진에서 호출되는 사망/실패 처리 오버라이드 함수
    virtual void FellOutOfWorld(const class UDamageType& DamageType) override;
    
    // 빠른 리스폰 처리 (스폰 위치 또는 최신 체크포인트 위치로 이동)
    void RespawnAtLastCheckpoint();

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
    
    // 벽 타기 시 낙하 속도 제한 (-150.0f = 천천히 미끄러짐)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallSlideSpeed = -150.0f;

    // 캐릭터 정면 기준 벽 감지 트레이스 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallTraceDistance = 60.0f;
    
    // 감지된 벽면의 노멀 벡터
    FVector CurrentWallNormal = FVector::ZeroVector;
    
    // Wall Jump시 벽 반대 방향으로 밀어내는 수평 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallJumpHorizontalImpulse = 500.0f;
    
    // Wall Jump시 위로 솟구치게 하는 수직 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallJumpVerticalImpulse = 500.0f;

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
    
    // Spark 연출 및 라이트 생성을 담당하는 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spark", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USparkComponent> SparkComponent;
    
    // Wall Slide Spark 방출 타이밍 조절용 변수
    float LastWallSlideSparkTime = 0.0f;
    
    // 착지 충돌 결과에서 실제 밟고있는 머티리얼 정보를 반환
    FHitResult ResolveLandingHit(const FHitResult& InHit) const;
    
    // 벽 슬라이드 마찰 스파크 주기적 트리거 보조 함수
    
};
