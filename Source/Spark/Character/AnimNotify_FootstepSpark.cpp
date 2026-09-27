#include "AnimNotify_FootstepSpark.h"
#include "Components/SkeletalMeshComponent.h"
#include "SparkCharacter.h"

void UAnimNotify_FootstepSpark::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (ASparkCharacter* SparkCharacter = MeshComp ? Cast<ASparkCharacter>(MeshComp->GetOwner()) : nullptr)
    {
        SparkCharacter->HandleFootstepNotify();
    }
}
