// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "OnlineSessionSettings.h"

#include "EOS_GameInstance.generated.h"

USTRUCT(BlueprintType)
struct FMyBlueprintSessionResult
{
	GENERATED_BODY()

	FOnlineSessionSearchResult SessionResult;
	
	UPROPERTY(BlueprintReadOnly)
	FString SessionId;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 OpenPrivateConnections;

	UPROPERTY(BlueprintReadOnly)
	int32 OpenPublicConnections;

	UPROPERTY(BlueprintReadOnly)
	FString ConnectionString;

	// Constructor to initialize properties
	FMyBlueprintSessionResult()
		: MaxPlayers(0)
		, OpenPrivateConnections(0)
		, OpenPublicConnections(0)
		, SessionId(TEXT(""))
		, ConnectionString(TEXT(""))
	{}
};

// Delegate який буде broadcast у BP
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFound, const TArray<FMyBlueprintSessionResult>&, Sessions);


UCLASS()
class THESTARTERS_API UEOS_GameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	IOnlineSubsystem* Subsystem;
	IOnlineIdentityPtr Identity;
	IOnlineSessionPtr Session;

	void OnLoginEOSCompleted(int32 LocalUserNum, bool Success, const FUniqueNetId& UserId, const FString& Error);
	void OnFindSessionsCompleted(bool isSuccesful);
	void OnFindSessionAndJoinCompleted(bool isSuccesful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void LoginWithEOS(FString ID, FString Token, FString LoginType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	FString GetPlayerUsername();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	bool IsPlayerLoggedIn();

	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	// DO NOT USE
	void FindSessionAndJoin();

	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	// This function finds max 20 sessions with IS_DEDICATED_SERVER = true parameter
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void JoinSession(FMyBlueprintSessionResult SessionToJoin);
	
	UPROPERTY(BlueprintAssignable, Category = "EOS Functions")
	FOnSessionsFound OnSessionsFound; // BP Event

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FMyBlueprintSessionResult CurrentSession;

	UPROPERTY(BlueprintReadWrite, Category = "Spawning")
	FString SelectedCharacterString;
};
