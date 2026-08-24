#include "Components/SparkComponent.h"

#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"

#include "Data/SparkEffectData.h"
#include "Data/SparkSurfaceData.h"
#include "Utility/SparkSurfaceLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

USparkComponent::USparkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;  // 현재는 Tick이 필요 없으므로 꺼둠
}

void USparkComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USparkComponent::SpawnSparkLight(const FVector& Location, float Intensity, float LightRadius, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APointLight* LightActor = World->SpawnActor<APointLight>(Location, FRotator::ZeroRotator, SpawnParams);
    if (!LightActor) return;
    
    // PointLightComponent를 가져와 조명 파라미터 설정
    UPointLightComponent* LightComponent = LightActor->PointLightComponent.Get();

    if (!LightComponent) return;
    
    // 런타임에 동적으로 켜지는 라이트이므로 Movable로 강제 설정
    LightComponent->SetMobility(EComponentMobility::Movable);
    LightComponent->SetIntensity(Intensity);
    LightComponent->SetAttenuationRadius(LightRadius);
    LightComponent->SetLightColor(DefaultLightColor);
    // 캐릭터 바로 옆에서 스폰되는 짧은 이펙트라, 그림자를 켜두면 캐릭터 자신의 그림자가 바닥에 드리워짐
    LightComponent->SetCastShadows(false);
    // 0.8~1.2초짜리 순간 플래시가 Lumen 간접광(GI)에 반영되면, Lumen이 그 빛을 캐시했다가
    // 서서히 지우는 과정에서 벽/바닥에 잔상처럼 남는 현상이 생김. 이 정도로 짧은 VFX는
    // 간접광 기여가 필요 없으므로 아예 제외해서 잔상을 원천 차단
    LightComponent->SetIndirectLightingIntensity(0.0f);

    // 잔광 페이드: Duration에 걸쳐 Intensity를 세제곱 커브로 감소시켜 "빠르게 감소" 느낌을 줌
    StartLightFadeOut(LightComponent, Intensity, Duration);
    LightActor->SetLifeSpan(Duration);
}

// 조명 밝기를 시간에 따라 세제곱 커브로 감쇠시키는 함수
void USparkComponent::StartLightFadeOut(UPointLightComponent* LightComponent, float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World || !LightComponent) return;
    
    const float StartIntensity = Intensity;
    const float StartTime = World->GetTimeSeconds();

    // 람다 실행 시점엔 조명이 이미 파괴됐을 수도 있어서 약참조로 들고 있음
    TWeakObjectPtr<UPointLightComponent> WeakLight = LightComponent;

    // 람다 안에서 자기 타이머(*FadeHandle)를 직접 해제해야 해서 SharedPtr로 핸들을 공유
    TSharedPtr<FTimerHandle> FadeHandle = MakeShared<FTimerHandle>();

    FTimerDelegate FadeDelegate = FTimerDelegate::CreateLambda([WeakLight, World, StartTime, StartIntensity, Duration, FadeHandle]()
    {
        // 조명이 이미 사라졌으면 타이머만 정리하고 종료
        if (!World || !WeakLight.IsValid())
        {
            if (World) World->GetTimerManager().ClearTimer(*FadeHandle);
            return;
        }

        // 경과 시간으로 진행률(0~1) 계산
        const float Elapsed = World->GetTimeSeconds() - StartTime;
        const float Alpha = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);

        // 처음엔 빨리 어두워지고 끝으로 갈수록 천천히 꺼지게 하는 커브
        const float FadeMultiplier = FMath::Pow(1.0f - Alpha, 3.0f);
        WeakLight->SetIntensity(StartIntensity * FadeMultiplier);

        // 다 끝났으면 타이머 해제
        if (Alpha >= 1.0f)
        {
            World->GetTimerManager().ClearTimer(*FadeHandle);
        }
    });
    
    // 0.03초마다 갱신해서 부드럽게 꺼지는 것처럼 보이게 함
    World->GetTimerManager().SetTimer(*FadeHandle, FadeDelegate, 0.03f, true);
}

bool USparkComponent::ApplySurfaceOverride(const FHitResult& HitResult, FSparkEffectData& InOutEffectData) const
{
    if (!SparkSurfaceDataAsset)
    {
        return true; // 표면 데이터 에셋이 없으면 기본 행동 FX 그대로 재생
    }

    const ESparkSurfaceType SurfaceType = USparkSurfaceLibrary::GetSurfaceType(HitResult);
    const FSparkSurfaceData& SurfaceData = SparkSurfaceDataAsset->GetSurfaceData(SurfaceType);

    // 스파크 발생이 비활성화된 표면이면 즉시 차단
    if (!SurfaceData.bCanGenerateSpark)
    {
        return false;
    }

    // 표면 전용 파티클이 지정되어 있으면 파티클 오버라이드
    if (SurfaceData.EffectData.ParticleSystem)
    {
        InOutEffectData.ParticleSystem = SurfaceData.EffectData.ParticleSystem;
    }

    // 표면 전용 조명 수치가 설정되어 있으면 조명 파라미터 오버라이드
    if (SurfaceData.EffectData.LightIntensity > 0.0f)
    {
        InOutEffectData.LightIntensity = SurfaceData.EffectData.LightIntensity;
        InOutEffectData.LightRadius = SurfaceData.EffectData.LightRadius;
        InOutEffectData.LightDuration = SurfaceData.EffectData.LightDuration;
    }

    return true;
}

void USparkComponent::ExecuteSparkFX(const FSparkEffectData& EffectData, const FVector& Location, const FVector& Normal, const FHitResult& HitResult)
{
    // 표면 검사 및 오버라이드
    FSparkEffectData FinalFXData = EffectData;
    if (!ApplySurfaceOverride(HitResult, FinalFXData)) return;
    // 조명은 파티클보다 더 높이 띄워서 바닥/캐릭터에 파묻히지 않게 함
    const FVector LightLocation = Location + (Normal * 15.0f);
    SpawnSparkLight(LightLocation, FinalFXData.LightIntensity, FinalFXData.LightRadius, FinalFXData.LightDuration);
    
    // 파티클 스폰
    if (FinalFXData.ParticleSystem && GetWorld())
    {
        const FRotator SparkRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FinalFXData.ParticleSystem, Location, SparkRotation);
    }
    
    // 디버그 로그 출력 (파티클 이름, 머티리얼/표면 정보, 조명 파라미터)
    const FString ParticleName = FinalFXData.ParticleSystem ? FinalFXData.ParticleSystem->GetName() : TEXT("None");
    const FString PhysMatName = HitResult.PhysMaterial.IsValid() ? HitResult.PhysMaterial->GetName() : TEXT("None");
  
    FString RenderMatName = TEXT("None");
    if (UPrimitiveComponent* HitComp = HitResult.GetComponent())
    {
        if (UMaterialInterface* Mat = HitComp->GetMaterial(0))
        {
            RenderMatName = Mat->GetName();
        }
    }
  
    const ESparkSurfaceType SurfaceType = USparkSurfaceLibrary::GetSurfaceType(HitResult);
    const FString SurfaceName = UEnum::GetValueAsString(SurfaceType);
  
    UE_LOG(LogTemp, Log, TEXT("[SparkFX] Particle: %s | Surface: %s | PhysMaterial: %s | RenderMaterial: %s | Intensity: %.1f | Radius: %.1f | Duration: %.2fs"),
        *ParticleName, *SurfaceName, *PhysMatName, *RenderMatName, FinalFXData.LightIntensity, FinalFXData.LightRadius, FinalFXData.LightDuration);
}

void USparkComponent::TriggerLandingSpark(const FHitResult& HitResult, float Fallspeed)
{
    const FVector Location = !HitResult.ImpactPoint.IsNearlyZero() ? HitResult.ImpactPoint : HitResult.Location;
    const FVector Normal = !HitResult.ImpactNormal.IsNearlyZero() ? HitResult.ImpactNormal : FVector::UpVector;
    
    // 바닥 메쉬 내부에 파묻혀 파티클/조명이 막히지 않도록 띄워주기 (WallSlide/WallJump와 동일하게 5unit)
    const FVector SpawnLocation = Location + (Normal * 5.0f);

    // 기본 fallback 값
    FLandingSparkEffectData FXData;
    FXData.LightIntensity = 6000.0f;
    FXData.MaxLightIntensity = 30000.0f;
    FXData.LightRadius = 500.0f;
    FXData.MaxLightRadius = 2000.0f;
    FXData.LightDuration = 0.8f;
    FXData.MaxLightDuration = 1.2f;
    
    // Data Asset이 연결되어 있으면 에셋의 설정값을 우선 적용
    if (SparkEffectDataAsset)
    {
        FXData = SparkEffectDataAsset->LandingData;
    }
    
    // 낙하 속도에 따라서 빛의 밝기, 범위, 지속시간을 조절함
    const float SpeedMagnitude = FMath::Abs(Fallspeed);
    // 일정 높이 이하는 기본 높이로 간주
    const float ExcessSpeed = FMath::Max(0.0f, SpeedMagnitude - 500.0f);
    
    // 조명 밝기: 기본 6000 ~ 최대 30000
    FXData.LightIntensity = FMath::Clamp(FXData.LightIntensity + (ExcessSpeed * 24.0f), FXData.LightIntensity, FXData.MaxLightIntensity);
    // 조명 범위: 기본 500 ~ 최대 2000
    FXData.LightRadius = FMath::Clamp(FXData.LightRadius + (ExcessSpeed * 1.5f), FXData.LightRadius, FXData.MaxLightRadius);
    // 조명 지속 시간: 낮은곳에서 낙하(약 0.8초) ~ 높은곳에서 낙하(최대 1.2초) 가변 조절
    FXData.LightDuration = FMath::Clamp(FXData.LightDuration + (ExcessSpeed * 0.00035f), FXData.LightDuration, FXData.MaxLightDuration);
    
    ExecuteSparkFX(FXData, SpawnLocation, Normal, HitResult);
}

void USparkComponent::TriggerWallSlideSpark(const FHitResult& HitResult)
{
    const FVector Location = !HitResult.ImpactPoint.IsNearlyZero() ? HitResult.ImpactPoint : HitResult.Location;
    const FVector Normal = !HitResult.ImpactNormal.IsNearlyZero() ? HitResult.ImpactNormal : FVector::ForwardVector;
    const FVector SpawnLocation = Location + (Normal * 5.0f);
    
    // 기본 Fallback 값
    FSparkEffectData FXData;
    FXData.LightIntensity = 4000.0f;
    FXData.LightRadius = 400.0f;
    FXData.LightDuration = 0.4f;
    
    // Data Asset이 연결되어 있으면 에셋의 설정값을 우선 적용
    if (SparkEffectDataAsset)
    {
        FXData = SparkEffectDataAsset->WallSlideData;
    }
    
    ExecuteSparkFX(FXData, SpawnLocation, Normal, HitResult);
}

void USparkComponent::TriggerWallJumpSpark(const FHitResult& HitResult)
{
    const FVector Location = !HitResult.ImpactPoint.IsNearlyZero() ? HitResult.ImpactPoint : HitResult.Location;
    const FVector Normal = !HitResult.ImpactNormal.IsNearlyZero() ? HitResult.ImpactNormal : FVector::ForwardVector;
    const FVector SpawnLocation = Location + (Normal * 5.0f);
    
    // 기본 fallback 값
    FSparkEffectData FXData;
    FXData.LightIntensity = 15000.0f;
    FXData.LightRadius = 1200.0f;
    FXData.LightDuration = 1.0f;
    
    // Data Asset이 연결되어 있으면 에셋의 설정값을 우선 적용
    if (SparkEffectDataAsset)
    {
        FXData = SparkEffectDataAsset->WallJumpData;
    }
    
    ExecuteSparkFX(FXData, SpawnLocation, Normal, HitResult);
}
