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

void APingMarker::RefreshVisuals()
{
    BP_OnPingTypeChanged(PingType);
}

void APingMarker::BeginPlay()
{
    Super::BeginPlay();

    // Initialize visuals for server + locally spawned instances.
    RefreshVisuals();
}

void APingMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Register a variable for replication
    // These are assigned right after spawn; using unconditional replication avoids edge cases
    // where InitialOnly could miss late assignment on some network paths.
    DOREPLIFETIME(APingMarker, TeamID);
    DOREPLIFETIME(APingMarker, PingType);
}

void APingMarker::OnRep_PingType()
{
    RefreshVisuals();
}

// Filtering: This code is executed ONLY ON THE SERVER for each client
bool APingMarker::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // RealViewer is the client PlayerController for which visibility is checked
    const APlayerController* PC = Cast<APlayerController>(RealViewer);

    if (PC && PC->PlayerState)
    {
        const AEOS_PlayerState* PS = Cast<AEOS_PlayerState>(PC->PlayerState);

        if (PS)
        {
            // Logic: Show ping only if commands match
            return PS->CurrentTeam == this->TeamID;
        }
    }

    // By default (if something goes wrong) it is better not to show or call Super
    return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

