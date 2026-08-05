#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SparkPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * ASparkPlayerController
 * 
 * 플레이어 입력 조작을 받아 캐릭터와 매핑하고, IMC 활성화를 관리하는 컨트롤러 클래스
 */
UCLASS()
class SPARK_API ASparkPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 기본 초기화 및 컴포넌트 설정
	ASparkPlayerController();

protected:
	// 로컬 플레이어 서브시스템을 통해 입력 매핑 컨텍스트(IMC_Default)를 활성화
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	// 플레이어 생성 시 기본으로 활성화할 입력 매핑 컨텍스트 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 캐릭터 상하좌우 이동에 사용하는 입력 액션 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	// 마우스 델타 시점 회전에 사용하는 입력 액션 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;
};
