#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SparkDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSparkDoorOpenedSignature);

/**
 * ASparkDoor
 * 
 * 상호작용 스위치나 퍼즐 장치에 의해 개폐되는 문 액터 클래스
 */
UCLASS()
class SPARK_API ASparkDoor : public AActor
{
	GENERATED_BODY()

public:	
	ASparkDoor();

    // 문이 열릴 때 방송. 레벨 블루프린트에서 바인딩해 조명 연출 등 레벨 고유 연출을 문 BP와 분리해 처리한다
    UPROPERTY(BlueprintAssignable, Category = "Door|Events")
    FSparkDoorOpenedSignature OnDoorOpened;

    // 문 열기
    UFUNCTION(BlueprintCallable, Category = "Door")
    virtual void OpenDoor();
    
    // 문 닫기
    UFUNCTION(BlueprintCallable, Category = "Door")
    virtual void CloseDoor();
    
    // 문 상태 토글
    UFUNCTION(BlueprintCallable, Category = "Door")
    virtual void ToggleDoor();
    
    // 현재 문이 열려있는지 체크
    UFUNCTION(BlueprintPure, Category = "Door")
    bool IsOpen() const { return bIsOpen; }

protected:
	virtual void BeginPlay() override;

    // 문이 열릴 때 블루프린트에서 애니메이션 처리
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
    void BP_OnDoorOpened();
    
    // 문이 닫힐 때 블루프린트에서 애니메이션 처리
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
    void BP_OnDoorClosed();
    
    // 문의 현재 상태
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    bool bIsOpen = false;
};
