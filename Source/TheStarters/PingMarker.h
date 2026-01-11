// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "EOS_PlayerState.h"
#include "PingTypes.h"

#include "PingMarker.generated.h"

UCLASS()
class THESTARTERS_API APingMarker : public AActor
{
	GENERATED_BODY()
	
public:
    APingMarker();

    // The Team ID this ping belongs to
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    ETeam TeamID = ETeam::None;

    UPROPERTY(ReplicatedUsing = OnRep_PingType, BlueprintReadOnly, Category = "Ping")
    EStarterPingType PingType = EStarterPingType::Location;

    UFUNCTION(BlueprintImplementableEvent, Category = "Ping")
    void BP_OnPingTypeChanged(EStarterPingType NewType);

    // Override this to filter visibility
    virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_PingType();
};
