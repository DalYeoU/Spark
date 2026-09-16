#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SparkSaveSubsystem.generated.h"

class USparkSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSparkSaveStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSparkSaveCompletedSignature, bool, bSuccess);

/**
 * USparkSaveSubsystem
 *
 * 세이브 및 체크포인트 영속성을 관리하는 서브시스템입니다.
 * GameInstance 수명 주기에 바인딩되어 레벨 전환 및 재시작 시에도 상태를 유지합니다.
 */
UCLASS()
class SPARK_API USparkSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USparkSaveSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 세이브 데이터 비동기 디스크 저장 시작 (완료/실패는 OnSaveCompletedGlobal 델리게이트로 통지)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SaveGameData(FName InCheckpointId, FName InLevelName, const FTransform& InPlayerTransform);

	// 저장이 진행 중인지 여부 (진행 중 종료/레벨 이동 방지 등에 사용)
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	bool IsSaving() const { return bIsSaving; }

	// Saving Indicator UI가 바인딩하는 전역 델리게이트 (저장 시작 시 브로드캐스트)
	static FSparkSaveStartedSignature OnSaveStartedGlobal;

	// Saving Indicator UI가 바인딩하는 전역 델리게이트 (저장 완료/실패 시 브로드캐스트)
	static FSparkSaveCompletedSignature OnSaveCompletedGlobal;

	// 디스크에서 세이브 데이터 로드 (실패 시 예외 처리 및 에러 로그 출력)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	USparkSaveGame* LoadGameData();

	// 저장된 세이브 파일이 존재하고 이어하기가 가능한지 검사
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	bool CanContinue() const;

	// 현재 메모리에 캐시된 세이브 데이터 반환
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	USparkSaveGame* GetCurrentSaveData() const { return CurrentSaveData; }

	// 세이브 슬롯 이름 반환
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	FString GetDefaultSlotName() const { return DefaultSlotName; }

	// 세이브 유저 인덱스 반환
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	int32 GetDefaultUserIndex() const { return DefaultUserIndex; }

	// 캐시된 세이브 데이터 초기화
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void ResetCurrentSaveData();

	// 다음 레벨 로드 시 체크포인트 위치로 복원해야 하는지 여부 확인
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	bool ShouldRestoreFromCheckpoint() const { return bShouldRestoreFromCheckpoint; }

	// 체크포인트 복원 플래그 설정 (재시작 또는 이어하기 전 활성화)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SetShouldRestoreFromCheckpoint(bool bEnable) { bShouldRestoreFromCheckpoint = bEnable; }

private:
	// 레벨 재시작 시 플레이어 위치 복원 트리거 플래그
	UPROPERTY(Transient)
	bool bShouldRestoreFromCheckpoint;

	// 비동기 저장 진행 중 여부
	UPROPERTY(Transient)
	bool bIsSaving = false;
	// 현재 로드/저장된 세이브 인스턴스 캐시
	UPROPERTY(Transient)
	TObjectPtr<USparkSaveGame> CurrentSaveData;

	// 기본 세이브 슬롯 이름
	UPROPERTY(EditDefaultsOnly, Category = "SaveSystem")
	FString DefaultSlotName;

	// 기본 유저 인덱스
	UPROPERTY(EditDefaultsOnly, Category = "SaveSystem")
	int32 DefaultUserIndex;
};
