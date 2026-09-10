#include "Save/SparkSaveGame.h"

USparkSaveGame::USparkSaveGame()
	: SaveSlotName(TEXT("SparkDefaultSaveSlot"))
	, UserIndex(0)
	, CheckpointId(NAME_None)
	, LevelName(NAME_None)
	, PlayerTransform(FTransform::Identity)
{
	// 기본값 초기화: 저장 전 슬롯 정보와 체크포인트/레벨 기본 상태 정의
}
