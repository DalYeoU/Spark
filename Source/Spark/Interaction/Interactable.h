#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Interactable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class SPARK_API IInteractable
{
	GENERATED_BODY()

public:
    // 상호작용 가능 여부 판별 (C++ 및 블루프린트 오버라이드 지원)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(APawn* InstigatorPawn) const;
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const;
    
    // 상호작용 실행 (C++ 및 블루프린트 오버라이드 지원)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(APawn* InstigatorPawn);
    virtual void Interact_Implementation(APawn* InstigatorPawn);
    
    // 프롬프트 UI에 표시할 텍스트 반환 (C++ 및 블루프린트 오버라이드 지원)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText GetInteractionText() const;
    virtual FText GetInteractionText_Implementation() const;
};
