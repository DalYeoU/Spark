#include "Checkpoint/SparkCheckpoint.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Save/SparkSaveSubsystem.h"

FOnCheckpointActivated ASparkCheckpoint::OnCheckpointActivatedGlobal;

ASparkCheckpoint::ASparkCheckpoint()
	: CheckpointId(NAME_None)
	, bAutoSaveOnOverlap(true)
	, bIsActivated(false)
	, ActiveLightIntensity(3000.0f)
	, FeedbackSpawnOffset(FVector(0.0f, 0.0f, 50.0f))
{
	// 틱 불필요 - 퍼포먼스 최적화
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 계층 구성
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));

	// 폰 오버랩만 감지하도록 콜리전 설정
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	RespawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(SceneRoot);

	// 활성화 시 점등될 상태 조명 컴포넌트
	ActiveLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ActiveLight"));
	ActiveLight->SetupAttachment(SceneRoot);
	ActiveLight->SetIntensity(0.0f); // 활성화 전에는 꺼둠
	ActiveLight->SetLightColor(FLinearColor(0.2f, 1.0f, 0.35f)); // 활성화/완료를 의미하는 Green (Accent Palette 기준)
	ActiveLight->SetCastShadows(false);
}

void ASparkCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 델리게이트 바인딩
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASparkCheckpoint::OnOverlapBegin);
	}
}

void ASparkCheckpoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이미 활성화된 체크포인트는 중복 처리 방지
	if (bIsActivated) return;

	// 플레이어가 제어하는 폰인지 검증
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled()) return;

	if (bAutoSaveOnOverlap)
	{
		ActivateCheckpoint(PlayerPawn);
	}
}

bool ASparkCheckpoint::ActivateCheckpoint(APawn* PlayerPawn)
{
	if (bIsActivated) return false;

	bIsActivated = true;

	// 세이브 서브시스템을 통해 체크포인트 데이터 저장
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return false;

	USparkSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USparkSaveSubsystem>();
	if (!SaveSubsystem) return false;

	const FName CurrentLevelName = *UGameplayStatics::GetCurrentLevelName(this, true);
	const FTransform RespawnTransform = GetRespawnTransform();

	const bool bSaveSuccess = SaveSubsystem->SaveGameData(CheckpointId, CurrentLevelName, RespawnTransform);

	// 상태 표시 라이트 점등
	if (ActiveLight)
	{
		ActiveLight->SetIntensity(ActiveLightIntensity);
	}

	const FVector FeedbackLocation = GetActorLocation() + FeedbackSpawnOffset;

	// 나이아가라 파티클 이펙트 스폰
	if (ActivationEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ActivationEffect, FeedbackLocation, GetActorRotation());
	}

	// 활성화 사운드 재생
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, FeedbackLocation);
	}

	// 블루프린트 연출/UI 피드백 이벤트 호출 (위젯 표시 등 추가 확장 지원)
	BP_OnCheckpointActivated();

	// UI Notice 등 외부 리스너에 활성화 알림 방송
	OnCheckpointActivatedGlobal.Broadcast(CheckpointId);

	return bSaveSuccess;
}

FTransform ASparkCheckpoint::GetRespawnTransform() const
{
	return RespawnPoint ? RespawnPoint->GetComponentTransform() : GetActorTransform();
}
