// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "PingTypes.h"

#include "PingWheelWidget.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class THESTARTERS_API UPingWheelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "PingWheel")
    void BP_SetHighlightedDirection(EPingWheelDirection Direction);
};


