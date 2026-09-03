#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "SparkSwitch.generated.h"

class ASparkDoor;

/**
 * ASparkSwitch
 * 
 * 플레이어 상호작용키를 통해 문이나 퍼즐 기믹을 활성화 하는 스위치 액터
 */
UCLASS()
class SPARK_API ASparkSwitch : public AActor , public IInteractable
{
	GENERATED_BODY()
	
public:	
	ASparkSwitch();
    
    // IInteractable 인터페이스 구현
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    virtual FText GetInteractionText_Implementation() const override;
    
protected:
	virtual void BeginPlay() override;

    // 스위치 활성화시 블루프린트 연출용 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Switch|Events")
    void BP_OnSwitchActivated();
    
    // 스위치 비활성화 블루프린트 연출용 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Switch|Events")
    void BP_OnSwitchDeactivated();
    
    // 스포이드 툴로 연결할 목표 문
    UPROPERTY(EditInstanceOnly, Category = "Interaction")
    TObjectPtr<ASparkDoor> TargetDoor;
    
    // 현재 스위치 활성화 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bIsActivated = false;
    
    // 다시 눌러서 끌 수 있는 토글형 스위치인지 체크
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bReusable = false;
    
    // 비활성화 상태에서 표시할 상호작용 문구
    UPROPERTY(EditInstanceOnly, Category = "Interaction")
    FText ActiveInteractionText;
    
    // 활성화 상태에서 표시할 상호작용 문구
    UPROPERTY(EditInstanceOnly, Category = "Interaction")
    FText InactiveInteractionText;

};
