#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SparkPauseMenuWidget.generated.h"

/**
* USparkPauseMenuWidget
*
* Pause 상태에서 표시되는 메뉴 위젯 클래스
* Resume / Restart from Checkpoint(확인 절차 포함) / Quit Game 버튼 클릭을 처리
* Settings / Controls / Return to Main Menu는 아직 구현되지 않아 WBP에서 비활성화 버튼으로만 배치
*/
UCLASS()
class SPARK_API USparkPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Resume 버튼 클릭 시 호출 (Pause 해제)
    UFUNCTION(BlueprintCallable, Category = "Pause|UI")
    void OnResumeClicked();

    // Restart from Checkpoint 버튼 클릭 시 호출 (확인 다이얼로그 노출)
    UFUNCTION(BlueprintCallable, Category = "Pause|UI")
    void OnRestartClicked();

    // 확인 다이얼로그에서 재시작을 확정했을 때 호출
    UFUNCTION(BlueprintCallable, Category = "Pause|UI")
    void OnRestartConfirmed();

    // 확인 다이얼로그에서 취소했을 때 호출
    UFUNCTION(BlueprintCallable, Category = "Pause|UI")
    void OnRestartCancelled();

    // Quit Game 버튼 클릭 시 호출
    UFUNCTION(BlueprintCallable, Category = "Pause|UI")
    void OnQuitClicked();

protected:
    // 블루프린트에서 확인 다이얼로그를 노출하도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Pause|UI")
    void BP_ShowRestartConfirm();

    // 블루프린트에서 확인 다이얼로그를 숨기도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Pause|UI")
    void BP_HideRestartConfirm();
};
