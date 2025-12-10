// Fill out your copyright notice in the Description page of Project Settings.


#include "PingMarker.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.h"

APingMarker::APingMarker()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = false;

    // IMPORTANT: Turn this off to force UE5 to ask for IsNetRelevantFor
    bAlwaysRelevant = false;
    bOnlyRelevantToOwner = false;
}

void APingMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Register a variable for replication
    // COND_InitialOnly is a good optimization if TeamID does not change after spawn.
    // If it can change, just use DOREPLIFETIME.
    DOREPLIFETIME_CONDITION(APingMarker, TeamID, COND_InitialOnly);
}

// Filtering: This code is executed ONLY ON THE SERVER for each client
bool APingMarker::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    UE_LOG(LogTemp, Error, TEXT("APingMarker::IsNetRelevantFor start"));

    // RealViewer is the client PlayerController for which visibility is checked
    const APlayerController* PC = Cast<APlayerController>(RealViewer);

    if (PC && PC->PlayerState)
    {
        UE_LOG(LogTemp, Error, TEXT("APingMarker::IsNetRelevantFor APlayerController cast"));

        const AEOS_PlayerState* PS = Cast<AEOS_PlayerState>(PC->PlayerState);

        if (PS)
        {
            UE_LOG(LogTemp, Error, TEXT("APingMarker::IsNetRelevantFor AEOS_PlayerState cast ... PS->CurrentTeam: %d, this->TeamID: %d"),
                (int32)PS->CurrentTeam,
                (int32)this->TeamID);
            // Logic: Show ping only if commands match
            return PS->CurrentTeam == this->TeamID;
        }
    }

    // By default (if something goes wrong) it is better not to show or call Super
    return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

