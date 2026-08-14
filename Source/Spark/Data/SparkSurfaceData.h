#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/SparkEffectData.h"

#include "SparkSurfaceData.generated.h"


UENUM(BlueprintType)
enum class ESparkSurfaceType : uint8
{
    Default     UMETA(DisplayName = "Default"),
    Metal       UMETA(DisplayName = "Metal"),
    Rubber      UMETA(DisplayName = "Rubber"),
    Cable       UMETA(DisplayName = "Cable"),
    None        UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FSparkSurfaceData
{
    GENERATED_BODY()
    
    // 해당 표면에서 Spark 발생을 허용할지 여부 (Rubber는 false)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
    bool bCanGenerateSpark = true;
    
    // 해당 표면에서 사용할 Spark 연출 데이터 (파티클, 조명 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface", meta = (EditCondition = "bCanGenerateSpark"))
    FSparkEffectData EffectData;
};

UCLASS(Blueprintable, BlueprintType)
class SPARK_API USparkSurfaceDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Surface")
    FSparkSurfaceData DefaultData;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Surface")
    FSparkSurfaceData MetalData;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Surface")
    FSparkSurfaceData RubberData;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Surface")
    FSparkSurfaceData CableData;
    
    const FSparkSurfaceData& GetSurfaceData(ESparkSurfaceType SurfaceType) const
    {
        switch (SurfaceType)
        {
        case ESparkSurfaceType::Metal: return MetalData;
        case ESparkSurfaceType::Rubber: return RubberData;
        case ESparkSurfaceType::Cable: return CableData;
        default: return DefaultData;
        }
    }
    
};
