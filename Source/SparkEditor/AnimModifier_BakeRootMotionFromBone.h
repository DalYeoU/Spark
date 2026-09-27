// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimationModifier.h"
#include "AnimModifier_BakeRootMotionFromBone.generated.h"

// 지정한 본의 수평 이동(X/Y)과 회전을 실제 root 본(인덱스 0)으로 옮기고, 원본 본에서는 그만큼 빼서 시각적 포즈는 그대로 유지한다
// root가 아닌 다른 본(예: pelvis)에 실제 이동/회전 데이터가 있는 리그에 사용한다
// Root Motion, Force Root Lock, ZeroOutRootBoneModifier는 전부 root 본(인덱스 0)만 읽고 써서 이런 리그에서는 조용히 아무 효과가 없다
UCLASS()
class UAnimModifier_BakeRootMotionFromBone : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName SourceBoneName = TEXT("pelvis");

	// 켜면 source 본의 world-space Yaw(회전 방향)만 root로 옮기고 Pitch/Roll/트위스트는 source에 그대로 남긴다
	// 제자리 회전 애니메이션에 사용한다. 쿼터니언 전체를 옮기면 자연스러운 몸 기울임(Pitch/Roll)까지 캐릭터 회전에 새어 들어가 옆으로 넘어지는 문제가 생긴다
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bYawOnlyRotation = false;

	// 켜면 source 본의 X/Y 이동만 0으로 버리고 회전은 전혀 건드리지 않는다
	// Root Motion을 계속 꺼둔 채 쓰는 애니메이션(예: 슬라이드 전환 몽타주)에 사용한다
	// Root Motion이 꺼져 있으면 root 본의 값도 그대로 렌더링에 반영되므로, 이동을 root로 옮기는 것만으로는 화면에서 사라지지 않는다
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bDiscardTranslationOnly = false;

	virtual void OnApply_Implementation(UAnimSequence* Animation) override;
};
