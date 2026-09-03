#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "SparkCableSocket.generated.h"

class ASparkDoor;
class ASparkCablePlug;
class USceneComponent;

/**
 * ASparkCableSocket
 * 
 * 케이블 플러그를 수신하여 전력을 공급하고 장치(문 등)를 활성화하는 소켓 액터
 */
UCLASS()
class SPARK_API ASparkCableSocket : public AActor, public IInteractable
{
    GENERATED_BODY()
	
public:	
    ASparkCableSocket();

    // IInteractable 인터페이스 구현
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    virtual FText GetInteractionText_Implementation() const override;

    // 플러그가 소켓에 꽂힐 때 호출
    UFUNCTION(BlueprintCallable, Category = "Cable Socket")
    virtual void PlugIn(ASparkCablePlug* InPlug);

    // 플러그가 소켓에서 분리될 때 호출
    UFUNCTION(BlueprintCallable, Category = "Cable Socket")
    virtual void Unplug();

    // 현재 전력 공급 상태 반환
    UFUNCTION(BlueprintPure, Category = "Cable Socket")
    bool IsPowered() const { return bIsPowered; }

    // 플러그가 스냅될 기준 위치 컴포넌트 반환
    USceneComponent* GetSocketAttachPoint() const { return SocketAttachPoint; }

protected:
    virtual void BeginPlay() override;

    // 전력 복구 시 블루프린트 연출용 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Cable Socket|Events")
    void BP_OnPowerRestored();

    // 전력 차단 시 블루프린트 연출용 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Cable Socket|Events")
    void BP_OnPowerBlocked();

protected:
    // 플러그가 결합될 스냅 위치 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Socket")
    TObjectPtr<USceneComponent> SocketAttachPoint;

    // 현재 결합된 플러그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Socket")
    TWeakObjectPtr<ASparkCablePlug> ConnectedPlug;

    // 전력이 복구되었을 때 열릴 문 (스포이트로 지정)
    UPROPERTY(EditInstanceOnly, Category = "Interaction")
    TObjectPtr<ASparkDoor> TargetDoor;

    // 전력 활성화 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Socket")
    bool bIsPowered = false;
};
