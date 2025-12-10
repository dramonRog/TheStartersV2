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

    // UPROPERTY(Replicated, BlueprintReadWrite, Category = "Spawning")
    UPROPERTY(BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<APawn> DesiredPawnClass;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player info")
    int PlayerKills = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player info")
    int PlayerDeath = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player info")
    bool isDead = false;

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeam(ETeam NewTeam);

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void AddKill();

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void AddDeath();

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void ChangeDeadState(bool NEWHEARTBEATSTATUS);

protected:
    UFUNCTION()
    void OnRep_CurrentTeam();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // virtual void CopyProperties(APlayerState* PlayerState) override;
};
