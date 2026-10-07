#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Save/SparkSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSparkSaveRoundTripTest, "Spark.Save.RoundTrip", EAutomationTestFlags::EditorContext |EAutomationTestFlags::ProductFilter)

bool FSparkSaveRoundTripTest::RunTest(const FString& Parameters)
{
	const FString TestSlot = TEXT("SparkAutomationTestSlot");
	UGameplayStatics::DeleteGameInSlot(TestSlot, 0);

	TestFalse(TEXT("없는 슬롯은 존재하지 않아야 한다"), UGameplayStatics::DoesSaveGameExist(TestSlot, 0));
	TestNull(TEXT("없는 슬롯 로드는 nullptr"), UGameplayStatics::LoadGameFromSlot(TestSlot, 0));

	USparkSaveGame* Save = Cast<USparkSaveGame>(UGameplayStatics::CreateSaveGameObject(USparkSaveGame::StaticClass()));
	if (!TestNotNull(TEXT("SaveGame 생성"), Save)) return false;

	TestEqual(TEXT("신규 객체의 SaveVersion 기본값은 0"), Save->SaveVersion, 0);
	TestTrue(TEXT("구버전(0) 세이브는 호환"), Save->IsCompatible());

	Save->SaveVersion = USparkSaveGame::CurrentSaveVersion;
	Save->CheckpointId = TEXT("CP_Test");
	Save->LevelName = TEXT("L_Test");
	Save->PlayerTransform = FTransform(FRotator(0.f, 90.f, 0.f), FVector(100.f, 200.f, 300.f));
	TestTrue(TEXT("저장 성공"), UGameplayStatics::SaveGameToSlot(Save, TestSlot, 0));

	const USparkSaveGame* Loaded = Cast<USparkSaveGame>(UGameplayStatics::LoadGameFromSlot(TestSlot, 0));
	if (TestNotNull(TEXT("로드 성공"), Loaded))
	{
		TestEqual(TEXT("SaveVersion 유지"), Loaded->SaveVersion, USparkSaveGame::CurrentSaveVersion);
		TestEqual(TEXT("CheckpointId 유지"), Loaded->CheckpointId, FName(TEXT("CP_Test")));
		TestEqual(TEXT("LevelName 유지"), Loaded->LevelName, FName(TEXT("L_Test")));
		TestTrue(TEXT("PlayerTransform 유지"), Loaded->PlayerTransform.Equals(Save->PlayerTransform));
	}

	Save->SaveVersion = USparkSaveGame::CurrentSaveVersion + 1;
	TestFalse(TEXT("더 새로운 버전은 비호환"), Save->IsCompatible());

	UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
	return true;
}

#endif
