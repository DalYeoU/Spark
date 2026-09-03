#include "Interaction/SparkDoor.h"

ASparkDoor::ASparkDoor()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ASparkDoor::BeginPlay()
{
	Super::BeginPlay();
}

void ASparkDoor::OpenDoor()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        BP_OnDoorOpened();
    }
}

void ASparkDoor::CloseDoor()
{
    if (bIsOpen)
    {
        bIsOpen = false;
        BP_OnDoorClosed();
    }
}

void ASparkDoor::ToggleDoor()
{
    if (bIsOpen) CloseDoor();
    else OpenDoor();
}
