// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameMode.h"
#include "EOS_PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

FString AEOS_GameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
    FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
    if (!ErrorMessage.IsEmpty())
    {
        return ErrorMessage;
    }

    // Parse "CharID" from Options
    FString CharacterID = UGameplayStatics::ParseOption(Options, TEXT("CharID"));

    // Find class in our Map
    TSubclassOf<APawn>* FoundClass = CharacterClassesMap.Find(CharacterID);

    // Write to PlayerState (hopefully)
    if (NewPlayerController)
    {
        AEOS_PlayerState* PS = NewPlayerController->GetPlayerState<AEOS_PlayerState>();
        if (PS)
        {
            PS->PlayerKills = 0;
            PS->PlayerDeath = 0;
            PS->isDead = false;

            if (FoundClass && *FoundClass)
            {
                PS->DesiredPawnClass = *FoundClass;
                UE_LOG(LogTemp, Log, TEXT("Player selected class: %s"), *CharacterID);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Character ID '%s' not found inside CharacterClassesMap! Using Default."), *CharacterID);
                PS->DesiredPawnClass = DefaultPawnClass;
            }
        }
    }

    return ErrorMessage;
}
