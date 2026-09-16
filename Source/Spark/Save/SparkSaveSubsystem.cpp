#include "Save/SparkSaveSubsystem.h"
#include "Save/SparkSaveGame.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSparkSave, Log, All);

FSparkSaveStartedSignature USparkSaveSubsystem::OnSaveStartedGlobal;
FSparkSaveCompletedSignature USparkSaveSubsystem::OnSaveCompletedGlobal;

USparkSaveSubsystem::USparkSaveSubsystem()
	: DefaultSlotName(TEXT("SparkDefaultSaveSlot"))
	, DefaultUserIndex(0)
	, bShouldRestoreFromCheckpoint(false)
{
}

void USparkSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSparkSave, Log, TEXT("SparkSaveSubsystem 초기화 완료"));
}

void USparkSaveSubsystem::Deinitialize()
{
	CurrentSaveData = nullptr;
	bShouldRestoreFromCheckpoint = false;
	Super::Deinitialize();
}

void USparkSaveSubsystem::SaveGameData(FName InCheckpointId, FName InLevelName, const FTransform& InPlayerTransform)
{
	// 기존 캐시가 없으면 새로 생성하여 세이브 데이터 준비
	if (!CurrentSaveData)
	{
		CurrentSaveData = Cast<USparkSaveGame>(UGameplayStatics::CreateSaveGameObject(USparkSaveGame::StaticClass()));
		if (!CurrentSaveData)
		{
			UE_LOG(LogSparkSave, Error, TEXT("SaveGameData 실패: SaveGame 오브젝트 생성 실패"));
			OnSaveCompletedGlobal.Broadcast(false);
			return;
		}
	}

	CurrentSaveData->SaveSlotName = DefaultSlotName;
	CurrentSaveData->UserIndex = DefaultUserIndex;
	CurrentSaveData->CheckpointId = InCheckpointId;
	CurrentSaveData->LevelName = InLevelName;
	CurrentSaveData->PlayerTransform = InPlayerTransform;

	bIsSaving = true;
	OnSaveStartedGlobal.Broadcast();

	// 비동기 디스크 저장. 완료 시점(성공/실패 모두)에 콜백에서 결과 통지
	FAsyncSaveGameToSlotDelegate SavedDelegate;
	SavedDelegate.BindWeakLambda(this, [this](const FString& SlotName, const int32 UserIndex, bool bSuccess)
	{
		bIsSaving = false;
		if (!bSuccess)
		{
			UE_LOG(LogSparkSave, Error, TEXT("SaveGameData 실패: 슬롯 '%s'(UserIndex: %d) 파일 저장 중 오류 발생"), *SlotName, UserIndex);
		}
		else
		{
			UE_LOG(LogSparkSave, Log, TEXT("SaveGameData 성공: 슬롯 '%s'(UserIndex: %d)"), *SlotName, UserIndex);
		}
		OnSaveCompletedGlobal.Broadcast(bSuccess);
	});
	UGameplayStatics::AsyncSaveGameToSlot(CurrentSaveData, DefaultSlotName, DefaultUserIndex, SavedDelegate);
}

USparkSaveGame* USparkSaveSubsystem::LoadGameData()
{
	// 슬롯 존재 여부 확인
	if (!UGameplayStatics::DoesSaveGameExist(DefaultSlotName, DefaultUserIndex))
	{
		UE_LOG(LogSparkSave, Warning, TEXT("LoadGameData: 슬롯 '%s'(UserIndex: %d)에 세이브 데이터가 존재하지 않습니다."), *DefaultSlotName, DefaultUserIndex);
		CurrentSaveData = nullptr;
		return nullptr;
	}

	// 디스크에서 로드 시도 및 실패 예외 처리
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(DefaultSlotName, DefaultUserIndex);
	if (!LoadedGame)
	{
		UE_LOG(LogSparkSave, Error, TEXT("LoadGameData 실패: 슬롯 '%s' 파일 로드 실패 (파일 손상 가능성)"), *DefaultSlotName);
		CurrentSaveData = nullptr;
		return nullptr;
	}

	// 타입 검증
	USparkSaveGame* SparkSave = Cast<USparkSaveGame>(LoadedGame);
	if (!SparkSave)
	{
		UE_LOG(LogSparkSave, Error, TEXT("LoadGameData 실패: 로드된 세이브 객체가 USparkSaveGame 타입이 아닙니다."));
		CurrentSaveData = nullptr;
		return nullptr;
	}

	CurrentSaveData = SparkSave;
	UE_LOG(LogSparkSave, Log, TEXT("LoadGameData 성공: Checkpoint [%s], Level [%s], Location [%s]"),
		*CurrentSaveData->CheckpointId.ToString(), *CurrentSaveData->LevelName.ToString(), *CurrentSaveData->PlayerTransform.GetLocation().ToString());

	return CurrentSaveData;
}

bool USparkSaveSubsystem::CanContinue() const
{
	// 디스크에 세이브 파일이 실제로 존재하는지 확인
	if (!UGameplayStatics::DoesSaveGameExist(DefaultSlotName, DefaultUserIndex)) return false;

	// 이미 메모리에 유효한 데이터가 로드되어 있는 경우
	if (CurrentSaveData && (!CurrentSaveData->LevelName.IsNone() || !CurrentSaveData->CheckpointId.IsNone())) return true;

	// 파일은 존재하지만 로드되지 않은 상태라면 가볍게 읽어 유효성 검증
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(DefaultSlotName, DefaultUserIndex);
	if (!LoadedGame)  return false;

	const USparkSaveGame* SparkSave = Cast<USparkSaveGame>(LoadedGame);
	if (!SparkSave) return false;

	// 레벨 이름이나 체크포인트 ID가 기록되어 있는지 확인
	return (!SparkSave->LevelName.IsNone() || !SparkSave->CheckpointId.IsNone());
}

void USparkSaveSubsystem::ResetCurrentSaveData()
{
	CurrentSaveData = nullptr;
}
