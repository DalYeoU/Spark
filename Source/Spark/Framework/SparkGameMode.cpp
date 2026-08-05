#include "SparkGameMode.h"

#include "Character/SparkCharacter.h"
#include "Framework/SparkPlayerController.h"

ASparkGameMode::ASparkGameMode()
{
    // 게임 시작 시 스폰할 기본 플레이어 캐릭터 클래스 지정
    DefaultPawnClass = ASparkCharacter::StaticClass();

    // 입력과 IMC 매핑을 담당할 기본 플레이어 컨트롤러 클래스 지정
    PlayerControllerClass = ASparkPlayerController::StaticClass();
}
