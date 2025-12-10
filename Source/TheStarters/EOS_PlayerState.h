// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EOS_PlayerState.generated.h"

// Forward Declaration to avoid including a heavy character file here
class ABaseCharacter;

UENUM(BlueprintType)
enum class ETeam : uint8
{
    None        UMETA(DisplayName = "None"),
    TeamA       UMETA(DisplayName = "Red Team"),
    TeamB       UMETA(DisplayName = "Blue Team")
};

UCLASS()
class THESTARTERS_API AEOS_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentTeam, BlueprintReadOnly, Category = "Team")
    ETeam CurrentTeam;

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeam(ETeam NewTeam);

    // UPROPERTY(Replicated, BlueprintReadWrite, Category = "Spawning")
    UPROPERTY(BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<APawn> DesiredPawnClass;

protected:
    UFUNCTION()
    void OnRep_CurrentTeam();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // virtual void CopyProperties(APlayerState* PlayerState) override;
};
