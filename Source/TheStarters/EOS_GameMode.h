// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EOS_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class THESTARTERS_API AEOS_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	// Map for converting ID (string) -> Class (BP)
	// Fill this in the GameMode Blueprint (for example: "Heavy" -> BP_HeavyCharacter)
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TMap<FString, TSubclassOf<APawn>> CharacterClassesMap;
};
