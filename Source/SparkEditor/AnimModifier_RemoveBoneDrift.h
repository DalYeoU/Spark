// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimationModifier.h"
#include "AnimModifier_RemoveBoneDrift.generated.h"

// 지정한 본이 클립 시작과 끝 사이에 쌓는 순 회전/이동(드리프트)을 제거해 제자리 포즈로 만든다
// 프레임별 상대 움직임(반동, 기울임 등)은 그대로 남기고, 시작~끝 누적분만 프레임 비율만큼 나눠서 역보정한다
// Root Motion 없이 애니메이션 자체가 특정 방향을 주장하지 않게 만들어야 할 때 사용한다(예: 물리적으로 다른 방향에서도 재생하는 액션 클립)
UCLASS()
class UAnimModifier_RemoveBoneDrift : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName SourceBoneName = TEXT("pelvis");

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bRemoveRotationDrift = true;

	// Z(수직)는 클립 고유의 점프 궤적이라 건드리지 않고 X/Y 수평 이동만 제거한다
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bRemoveTranslationDrift = true;

	// pelvis 포즈의 절대 방향이 캡슐 정면 기준으로 일정하게 틀어져 있을 때 모든 프레임에 동일하게 더해서 보정한다
	// 드리프트 제거(시작~끝 델타를 프레임 비율로 차감)와 달리 매 프레임 똑같은 각도를 더한다. 좌측을 보고 있으면 양수를 넣는다
	UPROPERTY(EditAnywhere, Category = "Settings")
	float ConstantYawOffsetDegrees = 0.0f;

	virtual void OnApply_Implementation(UAnimSequence* Animation) override;
};
