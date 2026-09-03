#include "Utility/SparkSurfaceLibrary.h"

#include "Kismet/GameplayStatics.h"

ESparkSurfaceType USparkSurfaceLibrary::GetSurfaceType(const FHitResult& HitResult)
{
	// HitResult로부터 물리 머티리얼 기반의 EPhysicalSurface 추출
	const EPhysicalSurface PhysicalSurface = UGameplayStatics::GetSurfaceType(HitResult);
	return ConvertToSparkSurfaceType(PhysicalSurface);
}

ESparkSurfaceType USparkSurfaceLibrary::ConvertToSparkSurfaceType(EPhysicalSurface SurfaceType)
{
	switch (SurfaceType)
	{
	case SurfaceType1: // Project Settings에 등록된 Metal
		return ESparkSurfaceType::Metal;

	case SurfaceType2: // Project Settings에 등록된 Rubber
		return ESparkSurfaceType::Rubber;

	case SurfaceType3: // Project Settings에 등록된 Cable
		return ESparkSurfaceType::Cable;

	case SurfaceType_Default:
	default:
		return ESparkSurfaceType::Default;
	}
}
