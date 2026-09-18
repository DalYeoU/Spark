#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SparkFailureMessageWidget.generated.h"

class UTextBlock;

/**
* USparkFailureMessageWidget
*
* 실패(낙사/HazardZone) 시 짧게 노출되는 안내 문구 위젯 클래스
*/
UCLASS()
class SPARK_API USparkFailureMessageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 문구를 설정하고 노출 애니메이션을 재생
    UFUNCTION(BlueprintCallable, Category = "Failure|UI")
    void ShowMessage(const FText& Message);

    // 소멸 애니메이션을 재생하고 숨김
    UFUNCTION(BlueprintCallable, Category = "Failure|UI")
    void HideMessage();

protected:
    virtual void NativeConstruct() override;

    // 블루프린트에서 등장 애니메이션을 재생할 수 있도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Failure|UI")
    void BP_OnShowMessage();

    // 블루프린트에서 소멸 애니메이션을 재생할 수 있도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Failure|UI")
    void BP_OnHideMessage();

    // WBP의 텍스트 블록과 자동으로 바인딩되는 텍스트 프로퍼티
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> MessageText;
};
