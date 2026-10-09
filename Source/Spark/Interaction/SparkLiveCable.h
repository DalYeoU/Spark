#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SparkLiveCable.generated.h"

class UBoxComponent;
class ASparkCharacter;

/**
 * ASparkLiveCable
 *
 * 천장에서 늘어지거나 바닥에 놓인 환경 케이블 액터입니다. 통과할 수 있고, 플레이어가 닿으면 닿은 위치에서 Cable Spark가 발생합니다.
 * 시각 메시는 이 액터를 상속한 블루프린트에서 SparkTrigger 아래에 붙입니다. 메시 충돌은 꺼야 통과됩니다.
 */
UCLASS()
class SPARK_API ASparkLiveCable : public AActor
{
    GENERATED_BODY()

public:
    ASparkLiveCable();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 플레이어 접촉 시작/종료 감지
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 겹쳐 있는 캐릭터와 가장 가까운 케이블 표면 위치에서 Spark를 발생
    void EmitSpark();

    // 접촉 감지 박스. 천장 케이블은 세로로 길게, 바닥 케이블은 얇고 납작하게 크기를 맞춘다
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live Cable")
    TObjectPtr<UBoxComponent> SparkTrigger;

    // 겹쳐 있는 동안 Spark를 반복하는 간격(초). 바닥 케이블을 따라 달릴 때도 계속 터지게 한다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Live Cable")
    float RepeatInterval = 0.25f;

private:
    // 현재 케이블에 닿아 있는 캐릭터
    TWeakObjectPtr<ASparkCharacter> OverlappingCharacter;

    // 접촉 중 Spark 반복용 타이머
    FTimerHandle TimerHandle_Repeat;
};
