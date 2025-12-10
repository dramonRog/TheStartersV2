// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.h"

void AEOS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEOS_PlayerState, CurrentTeam);
    // DOREPLIFETIME(AEOS_PlayerState, DesiredPawnClass);
}

void AEOS_PlayerState::SetTeam(ETeam NewTeam)
{
    if (HasAuthority())
    {
        CurrentTeam = NewTeam;

        // IMPORTANT: RepNotify is NOT called automatically on the server.
        // If we need to update the visual on the server (Host player) as well, we call it manually:
        if (GetNetMode() != NM_DedicatedServer)
        {
            OnRep_CurrentTeam();
        }
    }
}

void AEOS_PlayerState::OnRep_CurrentTeam()
{
    // This code is executed when the variable has already been updated!

    APawn* MyPawn = GetPawn();

    ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn);

    if (MyChar)
    {
        // Call the DIRECT visual update function (without RPC)
        MyChar->UpdateTeamVisuals(CurrentTeam);
    }
}

//void AEOS_PlayerState::CopyProperties(APlayerState* PlayerState)
//{
//    Super::CopyProperties(PlayerState);
//
//    UE_LOG(LogTemp, Log, TEXT("Start CopyProperties"));
//
//    AEOS_PlayerState* NewPlayerState = Cast<AEOS_PlayerState>(PlayerState);
//    if (NewPlayerState)
//    {
//        UE_LOG(LogTemp, Log, TEXT("COPING CopyProperties"));
//
//        // Transfer data
//        NewPlayerState->DesiredPawnClass = this->DesiredPawnClass;
//    }
//}