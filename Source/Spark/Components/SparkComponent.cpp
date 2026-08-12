#include "Components/SparkComponent.h"

#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "Data/SparkEffectData.h"

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
    
    // 잔광 페이드: Duration에 걸쳐 Intensity를 세제곱 커브로 감소시켜 "빠르게 감소" 느낌을 줌
    StartLightFadeOut(LightComponent, Intensity, Duration);
    LightActor->SetLifeSpan(Duration);
}

void USparkComponent::StartLightFadeOut(UPointLightComponent* LightComponent, float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World || !LightComponent) return;
    
    const float StartIntensity = Intensity;
    const float StartTime = World->GetTimeSeconds();
    TWeakObjectPtr<UPointLightComponent> WeakLight = LightComponent;
    TSharedPtr<FTimerHandle> FadeHandle = MakeShared<FTimerHandle>();
    
    FTimerDelegate FadeDelegate = FTimerDelegate::CreateLambda([WeakLight, World, StartTime, StartIntensity, Duration, FadeHandle]()
    {
        if (!World || !WeakLight.IsValid())
        {
            if (World) World->GetTimerManager().ClearTimer(*FadeHandle);
            return;
        }
        const float Elapsed = World->GetTimeSeconds() - StartTime;
        const float Alpha = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);
        const float FadeMultiplier = FMath::Pow(1.0f - Alpha, 3.0f);
        WeakLight->SetIntensity(StartIntensity * FadeMultiplier);

        if (Alpha >= 1.0f)
        {
            World->GetTimerManager().ClearTimer(*FadeHandle);
        }
    });
    
    World->GetTimerManager().SetTimer(*FadeHandle, FadeDelegate, 0.03f, true);
}

void USparkComponent::ExecuteSparkFX(const FSparkEffectData& EffectData, const FVector& Location, const FVector& Normal)
{
    // 조명 스폰
    SpawnSparkLight(Location, EffectData.LightIntensity, EffectData.LightRadius, EffectData.LightDuration);
    
    // 파티클 스폰
    if (EffectData.ParticleSystem && GetWorld())
    {
        const FRotator SparkRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectData.ParticleSystem, Location, SparkRotation);
    }
}

void USparkComponent::TriggerLandingSpark(const FVector& Location, const FVector& Normal, float Fallspeed)
{
    // 바닥 메쉬 내부에 파묻히지 않게 표면에 살짝 띄워줌
    const FVector SpawnLocation = Location + (Normal * 1.0f);
    
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
    
    ExecuteSparkFX(FXData, SpawnLocation, Normal);
}

void USparkComponent::TriggerWallSlideSpark(const FVector& Location, const FVector& WallNormal)
{
    const FVector SpawnLocation = Location + (WallNormal * 5.0f);
    
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
    
    ExecuteSparkFX(FXData, SpawnLocation, WallNormal);
}

void USparkComponent::TriggerWallJumpSpark(const FVector& Location, const FVector& WallNormal)
{
    const FVector SpawnLocation = Location + (WallNormal * 10.0f);
    
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
    
    ExecuteSparkFX(FXData, SpawnLocation, WallNormal);
}
