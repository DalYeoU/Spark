#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SparkSaveGame.generated.h"

/**
 * USparkSaveGame
 *
 * 체크포인트 ID, 레벨 정보, 플레이어 트랜스폼을 직렬화하여 저장하는 세이브 게임 클래스입니다.
 */
UCLASS()
class SPARK_API USparkSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 세이브 포맷 버전. 필드를 추가/변경할 때마다 올린다
	static constexpr int32 CurrentSaveVersion = 1;

	USparkSaveGame();

	// 이 세이브가 기록된 포맷 버전. 버전 필드 도입 전 세이브는 0으로 로드된다
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Meta")
	int32 SaveVersion;

	// 현재 빌드가 읽을 수 있는 세이브인지 검사한다. 더 새로운 버전으로 저장된 파일은 거부한다
	bool IsCompatible() const { return SaveVersion <= CurrentSaveVersion; }

	// 기본 세이브 슬롯 이름
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Meta")
	FString SaveSlotName;

	// 세이브 슬롯의 유저 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Meta")
	int32 UserIndex;

	// 마지막으로 도달하여 활성화된 체크포인트 식별자
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Checkpoint")
	FName CheckpointId;

	// 세이브 시점의 레벨 이름
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Level")
	FName LevelName;

	// 세이브 시점의 플레이어 위치 및 회전 트랜스폼
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	FTransform PlayerTransform;
};
