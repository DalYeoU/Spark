// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimModifier_BakeRootMotionFromBone.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/Skeleton.h"
#include "EngineLogs.h"

void UAnimModifier_BakeRootMotionFromBone::OnApply_Implementation(UAnimSequence* Animation)
{
	if (Animation == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_BakeRootMotionFromBone failed. Reason: Invalid Animation"));
		return;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (Model == nullptr || !Model->IsValidBoneTrackName(SourceBoneName))
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_BakeRootMotionFromBone failed. Reason: Bone '%s' has no track. Animation: %s"), *SourceBoneName.ToString(), *GetNameSafe(Animation));
		return;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton ? Skeleton->GetReferenceSkeleton() : FReferenceSkeleton();
	if (RefSkeleton.GetNum() == 0)
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_BakeRootMotionFromBone failed. Reason: Invalid Ref Skeleton. Animation: %s"), *GetNameSafe(Animation));
		return;
	}

	// SourceBoneName이 실제 root 본(인덱스 0)의 직계 자식일 때만 유효하다
	// 둘 사이에 이동값을 옮겨도 최종 world-space 포즈가 바뀌면 안 되기 때문이다
	const FName RootBoneName = RefSkeleton.GetBoneName(0);
	const int32 SourceBoneIndex = RefSkeleton.FindBoneIndex(SourceBoneName);
	if (RefSkeleton.GetParentIndex(SourceBoneIndex) != 0)
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_BakeRootMotionFromBone failed. Reason: '%s' is not a direct child of the root bone. Animation: %s"), *SourceBoneName.ToString(), *GetNameSafe(Animation));
		return;
	}

	const int32 NumKeys = Model->GetNumberOfKeys();
	const bool bShouldTransact = false;
	Controller.OpenBracket(FText::FromString(TEXT("Bake Root Motion From Bone")), bShouldTransact);

	// Bind Pose 자체가 회전되어 있는 리그라 0프레임의 절대 Yaw가 0이 아닐 수 있다
	// 그대로 두면 Root Motion이 몽타주 재생 첫 틱에 "이전 상태(0) → 0프레임 절대값"을 델타로 적용해버린다
	// 0프레임 값을 모든 프레임에서 빼서 root 회전이 항상 0프레임에 정확히 Identity로 시작하게 만든다
	float BaseYawDegrees = 0.0f;
	bool bHasBaseYaw = false;

	for (int32 AnimKey = 0; AnimKey < NumKeys; AnimKey++)
	{
		const FInt32Range KeyRangeToSet(AnimKey, AnimKey + 1);

		FTransform RootTransform = Model->EvaluateBoneTrackTransform(RootBoneName, AnimKey, EAnimInterpolationType::Step);
		FTransform SourceTransform = Model->EvaluateBoneTrackTransform(SourceBoneName, AnimKey, EAnimInterpolationType::Step);

		if (bDiscardTranslationOnly)
		{
			// 회전은 그대로 두고 이동(X/Y)만 버린다. root 본은 건드리지 않는다
			FVector SourceLocation = SourceTransform.GetLocation();
			SourceLocation.X = 0.0;
			SourceLocation.Y = 0.0;
			SourceTransform.SetLocation(SourceLocation);

			Controller.UpdateBoneTrackKeys(SourceBoneName, KeyRangeToSet, { SourceTransform.GetLocation() }, { SourceTransform.GetRotation() }, { SourceTransform.GetScale3D() });
			continue;
		}

		if (bYawOnlyRotation)
		{
			// 제자리 회전용 클립은 순 이동량이 0이어야 한다
			// source에 남아있는 미세한 스텝(체중 이동) 이동은 root로 옮기지 않고 그냥 버린다
			// 옮기면 Root Motion이 그걸 실제 캡슐 이동으로 적용해버린다
			FVector RootLocation = RootTransform.GetLocation();
			RootLocation.X = 0.0;
			RootLocation.Y = 0.0;
			RootTransform.SetLocation(RootLocation);

			FVector SourceLocation = SourceTransform.GetLocation();
			SourceLocation.X = 0.0;
			SourceLocation.Y = 0.0;
			SourceTransform.SetLocation(SourceLocation);
		}
		else
		{
			FVector NewRootLocation = RootTransform.GetLocation();
			NewRootLocation.X = SourceTransform.GetLocation().X;
			NewRootLocation.Y = SourceTransform.GetLocation().Y;
			RootTransform.SetLocation(NewRootLocation);

			FVector NewSourceLocation = SourceTransform.GetLocation();
			NewSourceLocation.X = 0.0;
			NewSourceLocation.Y = 0.0;
			SourceTransform.SetLocation(NewSourceLocation);
		}

		if (bYawOnlyRotation)
		{
			// Source의 world-space 회전 중 "Z축을 기준으로 얼마나 돌았는지"(Yaw)만 뽑아서 root로 옮긴다
			// SourceQuat.RotateVector(ForwardVector)의 결과를 수평면에 투영해 각도를 재는 방식이라, 본 자체의 로컬 축 관례(Reference 회전 90/90/90 등)와 무관하게 항상 정확하다
			const FQuat SourceQuat = SourceTransform.GetRotation();
			const FVector TurnedForward = SourceQuat.RotateVector(FVector::ForwardVector);
			const float AbsoluteYawDegrees = FMath::RadiansToDegrees(FMath::Atan2(TurnedForward.Y, TurnedForward.X));
			if (!bHasBaseYaw)
			{
				BaseYawDegrees = AbsoluteYawDegrees;
				bHasBaseYaw = true;
			}
			const float YawDegrees = FRotator::NormalizeAxis(AbsoluteYawDegrees - BaseYawDegrees);
			const FQuat YawOnlyQuat = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();

			RootTransform.SetRotation(YawOnlyQuat);
			// 나머지(피치/롤/트위스트)는 그대로 source에 남겨서 시각적 포즈는 원본과 동일하게 유지한다
			// 즉 Root(Yaw) * Residual = 원본 SourceQuat이 항상 성립해야 한다
			// UE의 FTransform 합성 순서를 손으로 계산하면 실수하기 쉬우므로(뒤틀린 회전 → T-포즈), 엔진이 실제로 쓰는 GetRelativeTransform으로 구한다
			const FTransform RootWorld(YawOnlyQuat);
			const FTransform SourceWorld(SourceQuat);
			SourceTransform.SetRotation(SourceWorld.GetRelativeTransform(RootWorld).GetRotation());
		}
		else
		{
			// 캐릭터가 바라보는 방향(회전)도 pelvis 대신 root가 갖도록 옮긴다
			// 이 리그는 본의 로컬 축이 일반적인 Z=Yaw 관례를 따르지 않으므로(Reference 회전이 90/90/90 근처), Euler 성분을 골라내지 않고 쿼터니언 전체를 옮긴다
			// 축 관례와 무관하게 항상 정확하다
			const FQuat NewRootRotation = SourceTransform.GetRotation();
			RootTransform.SetRotation(NewRootRotation);
			SourceTransform.SetRotation(FQuat::Identity);
		}

		Controller.UpdateBoneTrackKeys(RootBoneName, KeyRangeToSet, { RootTransform.GetLocation() }, { RootTransform.GetRotation() }, { RootTransform.GetScale3D() });
		Controller.UpdateBoneTrackKeys(SourceBoneName, KeyRangeToSet, { SourceTransform.GetLocation() }, { SourceTransform.GetRotation() }, { SourceTransform.GetScale3D() });
	}

	Controller.CloseBracket(bShouldTransact);
}
