#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SparkSavingIndicatorWidget.generated.h"

class UTextBlock;

/**
* USparkSavingIndicatorWidget
*
* 세이브 진행 중/완료/실패 상태를 화면 모서리에 표시하는 위젯 클래스
*/

UCLASS()
class SPARK_API USparkSavingIndicatorWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // 위젯 생성 및 초기화 시 델리게이트 바인딩
    virtual void NativeConstruct() override;

    // 위젯 파괴 시 델리게이트 정리
    virtual void NativeDestruct() override;

    // 세이브 시작 이벤트 핸들러
    UFUNCTION()
    void HandleSaveStarted();

    // 세이브 완료/실패 이벤트 핸들러
    UFUNCTION()
    void HandleSaveCompleted(bool bSuccess);

    // 블루프린트에서 등장 애니메이션을 재생할 수 있도록 제공하는 이벤트 ("저장 중..." 표시 시점)
    UFUNCTION(BlueprintImplementableEvent, Category = "Save|UI")
    void BP_OnShowSaving();

    // 블루프린트에서 소멸 애니메이션을 재생할 수 있도록 제공하는 이벤트 (완료/실패 표시 후 사라짐)
    UFUNCTION(BlueprintImplementableEvent, Category = "Save|UI")
    void BP_OnSaveFinished(bool bSuccess);

    // WBP의 텍스트 블록과 자동으로 바인딩되는 텍스트 프로퍼티
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> StatusText;
};
