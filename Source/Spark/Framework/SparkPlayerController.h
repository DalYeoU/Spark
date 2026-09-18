#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SparkPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class USparkPauseMenuWidget;
class USparkFailureMessageWidget;

/**
 * ASparkPlayerController
 * 
 * 플레이어 입력 조작을 받아 캐릭터와 매핑하고, IMC 활성화를 관리하는 컨트롤러 클래스
 */
UCLASS()
class SPARK_API ASparkPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // 기본 초기화 및 컴포넌트 설정
    ASparkPlayerController();

    // Pause Menu 표시 여부를 전환 (World Time 정지/재개, Input Mode 전환 포함)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetPauseMenuVisible(bool bVisible);

    // 실패(낙사/HazardZone) 안내 문구 위젯 안전한 게터
    FORCEINLINE USparkFailureMessageWidget* GetFailureMessageWidget() const { return FailureMessageWidgetInstance; }

protected:
    // 로컬 플레이어 서브시스템을 통해 입력 매핑 컨텍스트(IMC_Default)를 활성화
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Pause 입력 발생 시 Pause Menu 표시 여부를 토글
    void TogglePauseMenu();

private:
    // 플레이어 생성 시 기본으로 활성화할 입력 매핑 컨텍스트 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // 캐릭터 상하좌우 이동에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveAction;

    // 마우스 델타 시점 회전에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> LookAction;

    // 캐릭터 점프 조작에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> JumpAction;

    // 캐릭터 상호작용 조작에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> InteractAction;

    // 캐릭터 달리기 조작에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> SprintAction;

    // 캐릭터 슬라이딩 조작에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> SlideAction;

    // Pause Menu 열기/닫기 조작에 사용하는 입력 액션 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> PauseAction;

    // 뷰포트에 생성할 체크포인트 알림 위젯 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UUserWidget> CheckpointNoticeWidgetClass;

    // 뷰포트에 생성할 세이브 인디케이터 위젯 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UUserWidget> SavingIndicatorWidgetClass;

    // 뷰포트에 생성할 Pause Menu 위젯 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<USparkPauseMenuWidget> PauseMenuWidgetClass;

    // 뷰포트에 생성할 실패 안내 문구 위젯 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<USparkFailureMessageWidget> FailureMessageWidgetClass;

    // 생성된 Pause Menu 위젯 인스턴스
    UPROPERTY(Transient)
    TObjectPtr<USparkPauseMenuWidget> PauseMenuWidgetInstance;

    // 생성된 실패 안내 문구 위젯 인스턴스
    UPROPERTY(Transient)
    TObjectPtr<USparkFailureMessageWidget> FailureMessageWidgetInstance;

    // 현재 Pause 상태 여부
    bool bIsPaused = false;
};
