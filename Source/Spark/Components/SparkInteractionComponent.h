#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SparkInteractionComponent.generated.h"

// 대상 변경 알림용 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionTargetChanged, AActor*, NewTarget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPARK_API USparkInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USparkInteractionComponent();
    
    // 상호작용 키 입력 시 호출되는 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PrimaryInteract();
    
    // 현재 상호작용 가능한 대상 액터 반환
    UFUNCTION(blueprintpure, Category = "Interaction")
    AActor* GetCurrentInteractableActor() const;
    
    // 상호작용 대상 변경 시 발생하는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionTargetChanged OnInteractionTargetChanged;

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // 타이머로 반복 호출될 탐지 함수
    void PerformInteractionCheck();
    
    // 실제 트레이스를 수행하고 감지된 액터를 반환하는 보조 함수
    AActor* FindBestInteractable() const;

private:
    // 탐지 사거리
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionDistance = 50.0f;
    
    // 탐지 구체 반지름
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRadius = 20.0f;
    
    // 탐지 검사 주기
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float CheckInterval = 0.1f;
    
    // 주기적 탐지용 타이머 핸들
    FTimerHandle TimerHandle_InteractionCheck;
    
    //현재 포커스된 상호작용 액터
    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> CurrentInteractableActor;
    
};
