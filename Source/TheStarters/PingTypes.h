// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PingTypes.generated.h"

UENUM(BlueprintType)
enum class EStarterPingType : uint8
{
    Location    UMETA(DisplayName = "Location"),
    Enemy       UMETA(DisplayName = "Enemy"),
    Defend      UMETA(DisplayName = "Defend"),
};

UENUM(BlueprintType)
enum class EPingWheelDirection : uint8
{
    None    UMETA(DisplayName = "None"),
    Up      UMETA(DisplayName = "Up"),
    Right   UMETA(DisplayName = "Right"),
    Down    UMETA(DisplayName = "Down"),
    Left    UMETA(DisplayName = "Left"),
};


