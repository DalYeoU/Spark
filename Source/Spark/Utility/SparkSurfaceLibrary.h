#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/SparkSurfaceData.h"
#include "SparkSurfaceLibrary.generated.h"

/**
 * Surface 판정 및 관련 공용 유틸리티 함수를 제공하는 라이브러리 클래스입니다.
 */
UCLASS()
class SPARK_API USparkSurfaceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 충돌(HitResult)로부터 표면의 ESparkSurfaceType을 판정하여 반환합니다.
	 * @param HitResult 라인 트레이스 또는 충돌 결과 정보
	 * @return 판정된 ESparkSurfaceType
	 */
	UFUNCTION(BlueprintPure, Category = "Spark|Surface")
	static ESparkSurfaceType GetSurfaceType(const FHitResult& HitResult);

	/**
	 * 언리얼 엔진의 EPhysicalSurface를 게임 전용 ESparkSurfaceType으로 변환합니다.
	 * @param SurfaceType 언리얼 엔진 EPhysicalSurface 열거형 값
	 * @return 매핑된 ESparkSurfaceType
	 */
	UFUNCTION(BlueprintPure, Category = "Spark|Surface")
	static ESparkSurfaceType ConvertToSparkSurfaceType(EPhysicalSurface SurfaceType);
};
