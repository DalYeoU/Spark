#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_FootstepSpark.generated.h"

// Run/Sprint 애니메이션의 발이 지면에 닿는 프레임에 배치. Sprint 중일 때만 SparkComponent의 스파크를 터뜨림
UCLASS()
class SPARK_API UAnimNotify_FootstepSpark : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
    virtual FString GetNotifyName_Implementation() const override { return TEXT("Footstep Spark"); }
#endif
};
