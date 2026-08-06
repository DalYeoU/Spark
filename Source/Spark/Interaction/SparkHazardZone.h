#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SparkHazardZone.generated.h"

class UBoxComponent;

/**
 * ASparkHazardZone
 *
 * 낭떠러지, 가시밭 등 플레이어 캐릭터 진입 시 사망 및 Respawn을 유발하는 실패 영역 액터입니다.
 */
UCLASS()
class SPARK_API ASparkHazardZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ASparkHazardZone();

protected:
	virtual void BeginPlay() override;

    // 플레이어 캐릭터 진입 감지 오버랩 이벤트 핸들러
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
private:
    // 실패 감지 박스 영역 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBoxComponent> TriggerBox;

};
