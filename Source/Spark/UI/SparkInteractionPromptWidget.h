#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SparkInteractionPromptWidget.generated.h"

class UTextBlock;
class USparkInteractionComponent;

/**
* USparkInteractionPromptWidget
* 
* 상호작용 대상 감지 시 화면에 키 안내 및 상호작용 문구를 표시하는 UI 위젯 클래스
*/

UCLASS()
class SPARK_API USparkInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    // 위젯 생성 및 초기화 시 델리게이트 바인딩
    virtual void NativeConstruct() override;
    
    // 위젯 파괴 시 델리게이트 정리
    virtual void NativeDestruct() override;
    
    // 상호작용 대상 변경 이벤트 핸들러
    UFUNCTION()
    void HandleInteractionTargetChanged(AActor* NewTarget);
    
    // 블루프린트에서 애니메이션을 재생할 수 있도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
    void BP_OnShowPrompt(const FText& PromText);
    
    // 블루프린트에서 애니메이션을 재생할 수 있도록 제공하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
    void BP_OnHidePrompt();
    
    // WBP의 텍스트 블록과 자동으로 바인딩되는 텍스트 프로퍼티
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> InteractionText;
    
private:
    // 바인딩된 상호작용 컴포넌트 약참조
    TWeakObjectPtr<USparkInteractionComponent> CachedInteractionComponent;
	
};
