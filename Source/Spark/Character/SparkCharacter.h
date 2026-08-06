#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SparkCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
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

    // Enhanced Input 입력값에 따른 캐릭터 이동 가공 및 입력 전달
    void Move(const FInputActionValue& Value);

    // Enhanced Input 마우스 델타값에 따른 컨트롤러 시점 회전 처리
    void Look(const FInputActionValue& Value);

    // 카메라 스프링암 컴포넌트 안전한 게터
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    // 팔로우 카메라 컴포넌트 안전한 게터
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

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
    void HandleLanded(const FHitResult& Hit);
    
    // 전방 벽면 감지 및 wall Slide 조건 검사
    void CheckWallSlide();

private:
    // 캐릭터와의 거리를 유지하고 벽 충돌 시 카메라를 당겨주는 스프링암 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    // 플레이어 뷰포트 시점을 제공하는 팔로우 카메라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;
    
    //현재 벽 타기 상태 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsWallSliding;
    
    // 벽 타기 시 낙하 속도 제한 (-150.0f = 천천히 미끄러짐)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallSlideSpeed = -150.0f;

    // 캐릭터 정면 기준 벽 감지 트레이스 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WallTraceDistance = 60.0f;
};
