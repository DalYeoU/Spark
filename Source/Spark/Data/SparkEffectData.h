#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SparkEffectData.generated.h"

class UNiagaraSystem;

/**
 * 개별 Spark 이벤트(Landing, Wall Slide, Wall Jump)의 연출 파라미터 구조체
 */
USTRUCT(BlueprintType)
struct FSparkEffectData
{
    GENERATED_BODY()

    // 조명 최소 밝기
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float LightIntensity = 6000.0f;
    
    // 조명 최소 도달 범위
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float LightRadius = 500.0f;
    
    // 조명 최소 유지 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float LightDuration = 0.8f;

    // 재생할 Niagara 파티클 에셋
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|VFX")
    TObjectPtr<UNiagaraSystem> ParticleSystem = nullptr;
};

/**
 * 착지 전용 가변 Spark 연출 파라미터 구조체
 */
USTRUCT(BlueprintType)
struct FLandingSparkEffectData : public FSparkEffectData
{
    GENERATED_BODY()
    
    // 조명 최대 밝기
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float MaxLightIntensity = 30000.0f;  
    
    // 조명 최대 도달 범위
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float MaxLightRadius = 2000.0f;
    
    // 조명 최대 유지 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light")
    float MaxLightDuration = 1.2f;
};

/**
 * 이벤트별 Spark 연출 파라미터를 통합 관리하는 데이터 에셋입니다.
 */
UCLASS(BlueprintType)
class SPARK_API USparkEffectDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 착지(Landing) Spark 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Events")
    FLandingSparkEffectData LandingData;

    // 벽 미끄러짐(Wall Slide) Spark 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Events")
    FSparkEffectData WallSlideData;

    // 벽 점프(Wall Jump) Spark 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Events")
    FSparkEffectData WallJumpData;

    // 케이블(Cable Interaction) Spark 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Events")
    FSparkEffectData CableData;
};
