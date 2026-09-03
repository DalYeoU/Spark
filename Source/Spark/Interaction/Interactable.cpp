#include "Interaction/Interactable.h"
#include "GameFramework/Pawn.h"

bool IInteractable::CanInteract_Implementation(APawn* InstigatorPawn) const
{
    return true;
}

void IInteractable::Interact_Implementation(APawn* InstigatorPawn)
{
}

FText IInteractable::GetInteractionText_Implementation() const
{
    return FText::GetEmpty();
}

