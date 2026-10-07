// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimModifier_RemoveBoneDrift.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "EngineLogs.h"

void UAnimModifier_RemoveBoneDrift::OnApply_Implementation(UAnimSequence* Animation)
{
	if (Animation == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_RemoveBoneDrift failed. Reason: Invalid Animation"));
		return;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (Model == nullptr || !Model->IsValidBoneTrackName(SourceBoneName))
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_RemoveBoneDrift failed. Reason: Bone '%s' has no track. Animation: %s"), *SourceBoneName.ToString(), *GetNameSafe(Animation));
		return;
	}

	const int32 NumKeys = Model->GetNumberOfKeys();
	if (NumKeys < 2)
	{
		UE_LOG(LogAnimation, Error, TEXT("AnimModifier_RemoveBoneDrift failed. Reason: Not enough keys. Animation: %s"), *GetNameSafe(Animation));
		return;
	}

	const FTransform FirstTransform = Model->EvaluateBoneTrackTransform(SourceBoneName, 0, EAnimInterpolationType::Step);
	const FTransform LastTransform = Model->EvaluateBoneTrackTransform(SourceBoneName, NumKeys - 1, EAnimInterpolationType::Step);

	// 시작 포즈 기준 마지막 프레임까지 쌓인 순 회전(월드 스페이스). 이 회전을 프레임 비율만큼 역으로 걷어낸다
	const FQuat DriftRotation = LastTransform.GetRotation() * FirstTransform.GetRotation().Inverse();

	FVector DriftTranslation = LastTransform.GetLocation() - FirstTransform.GetLocation();
	DriftTranslation.Z = 0.0;

	const bool bShouldTransact = false;
	Controller.OpenBracket(FText::FromString(TEXT("Remove Bone Drift")), bShouldTransact);

	for (int32 AnimKey = 0; AnimKey < NumKeys; AnimKey++)
	{
		const FInt32Range KeyRangeToSet(AnimKey, AnimKey + 1);
		const float Alpha = static_cast<float>(AnimKey) / static_cast<float>(NumKeys - 1);

		FTransform SourceTransform = Model->EvaluateBoneTrackTransform(SourceBoneName, AnimKey, EAnimInterpolationType::Step);

		if (bRemoveRotationDrift)
		{
			// 지금까지 쌓였어야 할 드리프트 비율만큼만 역회전을 적용해서 매 프레임을 시작 포즈 기준으로 되돌린다
			const FQuat CorrectionQuat = FQuat::Slerp(FQuat::Identity, DriftRotation.Inverse(), Alpha);
			SourceTransform.SetRotation(CorrectionQuat * SourceTransform.GetRotation());
		}

		if (!FMath::IsNearlyZero(ConstantYawOffsetDegrees))
		{
			const FQuat YawOffsetQuat = FRotator(0.0f, ConstantYawOffsetDegrees, 0.0f).Quaternion();
			SourceTransform.SetRotation(YawOffsetQuat * SourceTransform.GetRotation());
		}

		if (bRemoveTranslationDrift)
		{
			FVector NewLocation = SourceTransform.GetLocation();
			NewLocation -= DriftTranslation * Alpha;
			SourceTransform.SetLocation(NewLocation);
		}

		Controller.UpdateBoneTrackKeys(SourceBoneName, KeyRangeToSet, { SourceTransform.GetLocation() }, { SourceTransform.GetRotation() }, { SourceTransform.GetScale3D() });
	}

	Controller.CloseBracket(bShouldTransact);
}
