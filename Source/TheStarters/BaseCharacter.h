// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// Input system
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Camera
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Data table for weapons (row type ST_WeaponTableRow used by DT_WeaponList)
#include "Engine/DataTable.h"

// Team System (enum class ETeam)
#include "EOS_PlayerState.h"

#include "BaseCharacter.generated.h"

// DataTable row used by DT_WeaponList
USTRUCT(BlueprintType)
struct FST_WeaponTableRow : public FTableRowBase
{
    GENERATED_BODY()

    // Static mesh shown for the weapon (optional for you)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMesh> StaticMesh = nullptr;

    // Blueprint class to spawn for this weapon (e.g. BP_ShooterWeapon_Pistol)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> SpawnActor = nullptr;
};


UCLASS(Abstract) // робить клас абстрактним (не можна створити напряму)
class THESTARTERS_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ABaseCharacter();

    virtual void Tick(float DeltaTime) override;
    void LeaveSession(FName sessionName);
   
    // --- TEAM SYSTEM ---
    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTeamVisuals(ETeam NewTeam);

    // --- PING SYSTEM ---
    UPROPERTY(EditDefaultsOnly, Category = "Ping")
    TSubclassOf<class APingMarker> PingActorClass;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FPCamera;
    
    // --- TEAM SYSTEM ---
    UPROPERTY(EditDefaultsOnly, Category = "Team Visuals")
    UMaterialInterface* MaterialTeamA;

    UPROPERTY(EditDefaultsOnly, Category = "Team Visuals")
    UMaterialInterface* MaterialTeamB;

    // LOGIC IMPROVEMENT: What if PlayerState arrived later than the character?
    virtual void OnRep_PlayerState() override;
    
    // Enhanced Input Actions
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputMappingContext* PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* SpecialAbilityAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* PingAction;

    // Input functions
    void Move(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);
    void SpecialAbility(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void PerformPing(const FInputActionValue& Value);

    // Server RPC
    UFUNCTION(Server, Reliable)
    void Server_SpawnPing(FVector HitLocation, FVector HitNormal, ETeam PingTeam);

    UFUNCTION(Server, Reliable)
    void Server_SetMoveSpeed(float NewSpeed);

    UFUNCTION()
    void OnRep_MoveSpeed();

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_MoveSpeed, Category = "Stats")
    float MoveSpeed = 600.f;

    // Fix client's twitch
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DefaultMoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SprintMultiplier = 2.0f;

    bool bIsSprinting = false;

    FDelegateHandle DestroySessionDelegateHandle;
    void HandleDestroySessionCompleted(FName SessionName, bool bWasSuccessful);

    /* ================== WEAPONS ================== */

    // Row in DT_WeaponList that defines this character's default weapon
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
    FDataTableRowHandle DefaultWeaponRow;

    // Socket on the character mesh to attach the weapon to
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
    FName WeaponSocketName = TEXT("WeaponSocket");

    // Currently spawned weapon actor
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapons")
    AActor* CurrentWeapon = nullptr;

    // Spawns and attaches weapon defined by DefaultWeaponRow
    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void EquipDefaultWeapon();
};
