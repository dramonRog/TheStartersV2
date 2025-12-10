// Fill out your copyright notice in the Description page of Project Settings.

// This player controller is currently responsible for letting server know what pawn we want to play for
// Set this player controller only on maps that client will play (not main menu, loading screen...)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EOS_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THESTARTERS_API AEOS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	
};
