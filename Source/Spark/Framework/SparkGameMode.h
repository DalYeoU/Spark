#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SparkGameMode.generated.h"

/**
 * ASparkGameMode
 * 
 * Spark 프로젝트의 메인 게임 규칙 및 플레이어 스폰 관리를 담당하는 GameMode 클래스입니다.
 */
UCLASS()
class SPARK_API ASparkGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 기본 Pawn 및 PlayerController 클래스 타입을 기본값으로 설정
	ASparkGameMode();
};
