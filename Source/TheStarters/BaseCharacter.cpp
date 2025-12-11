// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerState.h"

#include "EOS_GameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

// Network
#include "Net/UnrealNetwork.h"

// Ping logic
#include "Kismet/KismetSystemLibrary.h" // For LineTrace
#include "PingMarker.h"

// Team system
#include "EOS_PlayerState.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- First person camera ---
    FPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
    FPCamera->SetupAttachment(GetMesh(), TEXT("head"));
    // FPCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Eye level
    FPCamera->bUsePawnControlRotation = true;

    // Movement does not affect rotation
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Body control from the controller
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Better character physisc
    GetCharacterMovement()->GravityScale = 2.0f;
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.7f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
    GetCharacterMovement()->GroundFriction = 8.0f;
    GetCharacterMovement()->MaxAcceleration = 2048.0f;
    GetCharacterMovement()->Mass = 120.f;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("BaseCharacter: V1.2 Initialized!"));

    ENetMode netMode = GetNetMode();

    FString netModeStr = "Net mode: ";

    switch (netMode)
    {
    case NM_Standalone:
        netModeStr += "NM_Standalone";
        break;
    case NM_DedicatedServer:
        netModeStr += "NM_DedicatedServer";
        break;
    case NM_ListenServer:
        netModeStr += "NM_ListenServer";
        break;
    case NM_Client:
        netModeStr += "NM_Client";
        break;
    case NM_MAX:
        netModeStr += "NM_MAX";
        break;
    default:
        break;
    }

    FString RoleStr;
    FString WorldName = GetWorld() ? GetWorld()->GetName() : "NoWorld";

    switch (GetLocalRole())
    {
    case ROLE_Authority:       RoleStr = "ROLE_Authority (Server)";        break;
    case ROLE_AutonomousProxy: RoleStr = "ROLE_AutonomousProxy (Local)";   break;
    case ROLE_SimulatedProxy:  RoleStr = "ROLE_SimulatedProxy (Other)";    break;
    default:                   RoleStr = "ROLE_Unknown";                   break;
    }

    FString DebugMsg = FString::Printf(TEXT("%s | %s | %s | %s"),
        *GetName(), *netModeStr, *RoleStr, *WorldName);

    /*GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, DebugMsg);
    GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, GetName());
    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMsg);*/

    // Change capsule color for local player
    if (GetName().Equals("BP_BaseCharacter_C_0") && GetCapsuleComponent())
    {
        UMaterialInstanceDynamic* DynMat = GetCapsuleComponent()->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Black);
        }

        GetCapsuleComponent()->ShapeColor = FColor::Black;
        GetCapsuleComponent()->MarkRenderStateDirty();
    }

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(PlayerMappingContext, 0);
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    // Give this character its default weapon (based on DefaultWeaponRow)
    EquipDefaultWeapon();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseCharacter::LeaveSession()
{
    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!Subsystem) return;

    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (!Session.IsValid()) return;
    
    UEOS_GameInstance* GameInstanceRef = Cast<UEOS_GameInstance>(GetWorld()->GetGameInstance());

    if (GameInstanceRef->CurrentSession.SessionResult.IsValid()) {
        FName sessionName = FName(GameInstanceRef->CurrentSession.SessionResult.GetSessionIdStr());

        DestroySessionDelegateHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
            FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroySessionCompleted));

        // Remove the session locally so its no longer referenced.
        if (!Session->DestroySession(sessionName))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to destroy session locally."));
            Session->ClearOnEndSessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
            DestroySessionDelegateHandle.Reset();
        }
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    
    if (PC)
    {
        PC->ClientTravel(TEXT("/Game/Maps/MainMenu"), TRAVEL_Absolute);
    }
}

void ABaseCharacter::HandleDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (Session.IsValid())
    {
        Session->ClearOnEndSessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
        DestroySessionDelegateHandle.Reset();
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session destroyed successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session."));
    }
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ABaseCharacter::StartSprint);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
        EnhancedInput->BindAction(SpecialAbilityAction, ETriggerEvent::Started, this, &ABaseCharacter::SpecialAbility);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInput->BindAction(PingAction, ETriggerEvent::Started, this, &ABaseCharacter::PerformPing);
    }
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABaseCharacter::Server_SetMoveSpeed_Implementation(float NewSpeed)
{
    MoveSpeed = NewSpeed;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ABaseCharacter::StartSprint(const FInputActionValue& Value)
{
    bIsSprinting = true;

    if (HasAuthority())
    {
        MoveSpeed = DefaultMoveSpeed * SprintMultiplier;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
    else
    {
        Server_SetMoveSpeed(DefaultMoveSpeed * SprintMultiplier);
    }
}

void ABaseCharacter::StopSprint(const FInputActionValue& Value)
{
    bIsSprinting = false;

    if (HasAuthority())
    {
        MoveSpeed = DefaultMoveSpeed;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
    else
    {
        Server_SetMoveSpeed(DefaultMoveSpeed);
    }
}

void ABaseCharacter::OnRep_MoveSpeed()
{
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseCharacter::SpecialAbility(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Log, TEXT("Special Ability triggered"));
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Special Ability triggered"));

    /*APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (PC && PC->PlayerState)
    {
        const FUniqueNetIdRepl& UniqueId = PC->PlayerState->GetUniqueId();
        if (UniqueId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("SpecialAbility: Client UniqueNetId: %s"), *UniqueId->ToString());
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("SpecialAbility: Client UniqueNetId OK"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SpecialAbility: Client UniqueNetId is INVALID."));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("SpecialAbility: Client UniqueNetId is INVALID."));
        }
    }*/
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxis = Value.Get<FVector2D>();

    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y * -1.f);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABaseCharacter, MoveSpeed);
}

/* ================== WEAPONS ================== */
void ABaseCharacter::EquipDefaultWeapon()
{
    // If no row configured, do nothing
    if (!DefaultWeaponRow.DataTable || DefaultWeaponRow.RowName.IsNone())
    {
        return;
    }

    // Find the row in the DataTable (DT_WeaponList)
    const FST_WeaponTableRow* Row =
        DefaultWeaponRow.DataTable->FindRow<FST_WeaponTableRow>(
            DefaultWeaponRow.RowName,
            TEXT("EquipDefaultWeapon"));

    if (!Row)
    {
        return; // Invalid row name
    }

    // SpawnActor must be set in the row (TSubclassOf<AActor>)
    if (!Row->SpawnActor)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    // Spawn the weapon actor
    AActor* Weapon = World->SpawnActor<AActor>(
        Row->SpawnActor,
        GetActorLocation(),
        GetActorRotation(),
        SpawnParams);

    if (!Weapon)
    {
        return;
    }

    CurrentWeapon = Weapon;

    // Attach it to the character mesh at the specified socket
    Weapon->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        WeaponSocketName);
}

/* ================== PING SYSTEM ================== */
void ABaseCharacter::PerformPing(const FInputActionValue& Value)
{
    // We only want the local controller to instigate the ping trace
    if (!IsLocallyControlled() || !FPCamera) return;

    FVector Start = FPCamera->GetComponentLocation();
    FVector Forward = FPCamera->GetForwardVector();
    FVector End = Start + (Forward * 10000.0f); // 100 meter trace

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // Ignore self

    // Perform Line Trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        ETeam myTeam = ETeam::None;
        AEOS_PlayerState* MyPS = GetPlayerState<AEOS_PlayerState>();
        if (MyPS)
        {
            myTeam = MyPS->CurrentTeam;
        }

        // Tell the server.
        Server_SpawnPing(HitResult.Location, HitResult.Normal, myTeam);
    }
    else
    {
        // Optional: Ping "in the air" at max distance
        Server_SpawnPing(End, FVector::UpVector, ETeam::None);
    }
}

void ABaseCharacter::Server_SpawnPing_Implementation(FVector HitLocation, FVector HitNormal, ETeam PingTeam)
{
    if (!PingActorClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    // Slight offset so the ping doesn't clip inside the wall
    FVector SpawnLoc = HitLocation + (HitNormal * 5.0f);
    FRotator SpawnRot = HitNormal.Rotation();

    APingMarker* NewPing = GetWorld()->SpawnActor<APingMarker>(PingActorClass, SpawnLoc, SpawnRot, SpawnParams);

    if (NewPing)
    {
        // Assign the TeamID to the Ping so it knows who to replicate to
        NewPing->TeamID = PingTeam;

        // Destroy the ping after 5 seconds
        NewPing->SetLifeSpan(5.0f);
    }
}

/* ================== TEAM SYSTEM ================== */
void ABaseCharacter::UpdateTeamVisuals(ETeam NewTeam)
{
    UMaterialInterface* TargetMaterial = nullptr;

    switch (NewTeam)
    {
    case ETeam::TeamA:
        TargetMaterial = MaterialTeamA;
        break;
    case ETeam::TeamB:
        TargetMaterial = MaterialTeamB;
        break;
    default:
        // Should return the default material
        break;
    }

    if (TargetMaterial && GetMesh())
    {
        GetMesh()->SetMaterial(0, TargetMaterial);
    }
}

// This is an improvement: it solves the "Race Condition" problem
void ABaseCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // When PlayerState finally syncs with this character,
    // we check what team it is on and immediately paint it.
    AEOS_PlayerState* MyPS = GetPlayerState<AEOS_PlayerState>();
    if (MyPS)
    {
        UpdateTeamVisuals(MyPS->CurrentTeam);
    }
}