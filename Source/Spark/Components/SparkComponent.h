#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SparkComponent.generated.h"

class UNiagaraSystem;
class USparkEffectDataAsset;
class UPointLightComponent;
struct FSparkEffectData;
/**
 * USparkComponent
 *
 * Spark 생성 및 관련 연출을 전담하는 컴포넌트입니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPARK_API USparkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USparkComponent();

	/**
	 * 지정된 위치에 임시 Point Light Actor를 동적으로 생성
	 * @param Location 조명이 생성될 위치
	 * @param Intensity 조명 밝기 (기본값: 5000.0f)
	 * @param LightRadius 조명 도달 범위 (기본값: 500.0f)
	 * @param Duration 조명 유지 시간(초) (기본값: 1.0f)
	 */
	UFUNCTION(BlueprintCallable, Category = "Spark|Light")
	void SpawnSparkLight(const FVector& Location, float Intensity = 5000.0f, float LightRadius = 500.0f, float Duration = 1.0f);
    
    /**
     * 캐릭터 착지 시 Landing Spark 연출 트리거
     * @param Location 착지 발생 월드 위치
     * @param Normal 착지 지점의 표면 법선 벡터 (불꽃이 튀는 반사 방향 정렬용)
     * @param Fallspeed 착지 직전 캐릭터의 낙하 속도 (연출 강도 조절용)
     */
    UFUNCTION(BlueprintCallable, Category = "Spark|Events")
    void TriggerLandingSpark(const FVector& Location, const FVector& Normal, float Fallspeed = 0.0f);
    
    /*
     * 벽 슬라이드 마찰 Spark 연출 트리거
     * @param Location 벽면 접촉 위치
     * @param WallNormal 감지된 벽면 법선
     */
    UFUNCTION(BlueprintCallable, Category = "Spark|Events")
    void TriggerWallSlideSpark(const FVector& Location, const FVector& WallNormal);
    
    /*
     * 벽 점프 순간 폭발 Spark 연출 트리거
     * @param Location 벽 점프 발생 위치
     * @param WallNormal 감지된 벽면 법선
     */
    UFUNCTION(BlueprintCallable, Category = "Spark|Events")
    void TriggerWallJumpSpark(const FVector& Location, const FVector& WallNormal);
    
protected:
	virtual void BeginPlay() override;

private:
	// 기본 조명 색상 (기본값: 따듯한 주황/전기색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Light", meta = (AllowPrivateAccess = "true"))
	FLinearColor DefaultLightColor = FLinearColor(1.0f, 0.6f, 0.2f);
    
    // 이벤트별 Spark 강도 및 에셋 파라미터를 통합 관리하는 데이터 에셋
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spark|Data", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USparkEffectDataAsset> SparkEffectDataAsset;
    
    // 공통 Spark FX(조명 + 파티클) 일괄 실행 보조 함수
    void ExecuteSparkFX(const FSparkEffectData& EffectData, const FVector& Location, const FVector& Normal);
    
    // Point Light 잔광 페이드 아웃 타이머 시작 보조 함수
    void StartLightFadeOut(UPointLightComponent* LightComponent, float Intensity, float Duration);
    
};
