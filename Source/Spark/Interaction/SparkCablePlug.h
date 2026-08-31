#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "SparkCablePlug.generated.h"

class ASparkCableSocket;
class UCableComponent;

/**
 * ASparkCablePlug
 * 
 * 플레이어가 들고 이동하여 소켓에 꽂을 수 있는 케이블 플러그 액터
 */
UCLASS()
class SPARK_API ASparkCablePlug : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ASparkCablePlug();

    // IInteractable 인터페이스 구현
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    virtual FText GetInteractionText_Implementation() const override;

    // 플러그 들기 / 내려놓기 / 꽂기
    virtual void PickUp(APawn* InstigatorPawn);
    virtual void Drop();
    virtual void PlugIntoSocket(ASparkCableSocket* TargetSocket);

protected:
    virtual void BeginPlay() override;

    // 근처에 꽂을 수 있는 유효한 소켓 탐색
    ASparkCableSocket* FindNearbySocket() const;

    // 플러그 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Plug")
    TObjectPtr<UStaticMeshComponent> PlugMesh;

    // 물리 케이블 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Plug")
    TObjectPtr<UCableComponent> CableComponent;

    // 현재 플러그를 들고 있는 캐릭터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Plug")
    TWeakObjectPtr<APawn> HoldingPawn;

    // 현재 꽂혀 있는 소켓
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable Plug")
    TWeakObjectPtr<ASparkCableSocket> AttachedSocket;

    // 소켓 감지 반경
    UPROPERTY(EditAnywhere, Category = "Cable Plug")
    float SocketSnapDistance = 150.0f;
};
