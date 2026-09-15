#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SparkCheckpoint.generated.h"

class UBoxComponent;
class USceneComponent;
class UNiagaraSystem;
class USoundBase;
class UPointLightComponent;

// 체크포인트 활성화를 UI 등 외부 리스너에 알리는 전역 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckpointActivated, FName, ActivatedCheckpointId);

/**
 * ASparkCheckpoint
 *
 * 플레이어가 통과하면 해당 위치를 마지막 체크포인트로 등록하고 세이브 시스템에 저장하는 액터입니다.
 */
UCLASS()
class SPARK_API ASparkCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ASparkCheckpoint();

	// 체크포인트 활성화 시 방송되는 전역 델리게이트 (UI Notice 등에서 구독)
	static FOnCheckpointActivated OnCheckpointActivatedGlobal;

	// 체크포인트를 수동 또는 외부에서 활성화
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool ActivateCheckpoint(APawn* PlayerPawn);

	// 체크포인트 활성화 시 블루프린트 연출/이벤트 확장용
	UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
	void BP_OnCheckpointActivated();

	// 리스폰 지점 트랜스폼 반환
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FTransform GetRespawnTransform() const;

	// 현재 활성화 여부 반환
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsActivated() const { return bIsActivated; }

	// 체크포인트 식별자 반환
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FName GetCheckpointId() const { return CheckpointId; }

protected:
	virtual void BeginPlay() override;

	// 플레이어 진입 감지 오버랩 델리게이트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// 플레이어 감지용 트리거 볼륨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// 플레이어가 리스폰될 위치 및 방향 기준점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint|Components")
	TObjectPtr<USceneComponent> RespawnPoint;

	// 활성화 상태를 나타내는 포인트 라이트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint|Components")
	TObjectPtr<UPointLightComponent> ActiveLight;

	// 체크포인트 고유 식별자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointId;

	// 오버랩 시 자동 세이브 실행 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bAutoSaveOnOverlap;

	// 이미 활성화되어 등록되었는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	bool bIsActivated;

	// 활성화 시 방출할 나이아가라 파티클 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Feedback")
	TObjectPtr<UNiagaraSystem> ActivationEffect;

	// 활성화 시 재생할 효과음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Feedback")
	TObjectPtr<USoundBase> ActivationSound;

	// 활성화 라이트의 목표 밝기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Feedback")
	float ActiveLightIntensity;

	// 피드백 연출 발생 위치 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Feedback")
	FVector FeedbackSpawnOffset;
};
