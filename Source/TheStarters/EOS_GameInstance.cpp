// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"      // For file reading/writing
#include "HAL/PlatformFilemanager.h" // For accessing the platform file system
#include "EOS_PlayerController.h"

void UEOS_GameInstance::Init()
{
	Super::Init();

	Subsystem = Online::GetSubsystem(this->GetWorld());
	if (!Subsystem) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	Identity = Subsystem->GetIdentityInterface();
	if (!Identity) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return;
	}

	Session = Subsystem->GetSessionInterface();
	if (!Session) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSessionPtr FAIL"));
		return;
	}

	// Clear delegates
	Identity->OnLoginCompleteDelegates->Clear();
	Session->OnFindSessionsCompleteDelegates.Clear();
	Session->OnJoinSessionCompleteDelegates.Clear();
	// Create delegates
	Identity->OnLoginCompleteDelegates->AddUObject(this, &UEOS_GameInstance::OnLoginEOSCompleted);
	Session->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionsCompleted);
	Session->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnJoinSessionCompleted);
}

void UEOS_GameInstance::LoginWithEOS(FString ID, FString Token, FString LoginType)
{
	// !!! Command line paramerts (-AUTH_TYPE ...) have priority over function parametrs

	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	// If you're logged in, don't try to login again.
	// This can happen if your player travels to a dedicated server or different maps
	// Make this method availble to call from BeginPlay many times

	FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);

	if (NetId != nullptr && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		return;
	}

	// Grab command line parameters. If empty call hardcoded login function - Hardcoded login function useful for Play In Editor. 
	FString AuthType;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);
	UE_LOG(LogTemp, Log, TEXT("AuthType: "), *AuthType);

	// Initiate login process
	if (!AuthType.IsEmpty()) //If parameter is NOT empty we can autologin.
	{
		/*
		In most situations you will want to automatically log a player in using the parameters passed via CLI.
		For example, using the exchange code for the Epic Games Store.
		*/
		UE_LOG(LogTemp, Log, TEXT("AutoLogging into EOS...")); // Log to the UE logs that we are trying to log in. 

		if (!Identity->AutoLogin(0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
		}
	}
	else
	{
		/*
		Fallback if the CLI parameters are empty. Useful for PIE.
		The type here could be developer if using the DevAuthTool, ExchangeCode if the game is launched via the Epic Games Launcher, etc...
		*/
		FOnlineAccountCredentials Credentials(LoginType, ID, Token);

		UE_LOG(LogTemp, Log, TEXT("Logging into EOS...")); // Log to the UE logs that we are trying to log in. 

		if (!Identity->Login(0, Credentials))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
		}
	}
}

void UEOS_GameInstance::OnLoginEOSCompleted(int32 LocalUserNum, bool Success, const FUniqueNetId& UserId, const FString& Error)
{
	if (Success) {
		UE_LOG(LogTemp, Warning, TEXT("Login Success - UserID: %s"), *UserId.ToString());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Login FAIL, Reason - %s"), *Error);
	}
}

FString UEOS_GameInstance::GetPlayerUsername()
{
	FString userName = "Null";

	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return userName;
	}

	FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);
	auto UserOnlineAccount = Identity->GetUserAccount(*NetId.ToSharedRef());

	// User id is in the format "external_account_id|product_user_id"
	userName = UserOnlineAccount->GetDisplayName();

	return userName;
}

bool UEOS_GameInstance::IsPlayerLoggedIn()
{
	if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn) {
		return true;
	}

	return false;
}

void UEOS_GameInstance::FindSessionAndJoin()
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	FName SearchKey = "KeyName";
	FString SearchValue = "KeyValue";

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// Remove the default search parameters that FOnlineSessionSearch sets up.
	SessionSearch->QuerySettings.SearchParams.Empty();

	SessionSearch->QuerySettings.Set(FName("IS_DEDICATED_SERVER"), true, EOnlineComparisonOp::Equals); // Seach using our Key/Value pair

	// SessionSearch->MaxSearchResults = 20;
	// SessionSearch->bIsLanQuery = false;

	Session->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionAndJoinCompleted);

	UE_LOG(LogTemp, Log, TEXT("Finding session."));

	Session->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOS_GameInstance::OnFindSessionAndJoinCompleted(bool isSuccesful)
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	FString ConnectString;
	FOnlineSessionSearchResult* SessionToJoin;

	if (isSuccesful) {
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted, Num of session found: %d"), SessionSearch->SearchResults.Num());

		for (auto SessionInSearchResult : SessionSearch->SearchResults)
		{
			// Typically you want to check if the session is valid before joining. There is a bug in the EOS OSS where IsValid() returns false when the session is created on a DS. 
			// Instead of customizing the engine for this tutorial, we're simply not checking if the session is valid. The code below should go in this if statement once the bug is fixed. 
			/*
			if (SessionInSearchResult.IsValid())
			{


		}
			*/

			//Ensure the connection string is resolvable and store the info in ConnectInfo and in SessionToJoin
			if (Session->GetResolvedConnectString(SessionInSearchResult, NAME_GamePort, ConnectString))
			{
				SessionToJoin = &SessionInSearchResult;
				UE_LOG(LogTemp, Error, TEXT("Found session id: %s, opened connections: %d"), *SessionToJoin->GetSessionIdStr(), SessionToJoin->Session.NumOpenPrivateConnections);
			}

			UE_LOG(LogTemp, Error, TEXT("Session connection string: %s"), *ConnectString);
		}

		// Test Join for first avaible session
		if (SessionSearch->SearchResults.Num() > 0) {
			UE_LOG(LogTemp, Warning, TEXT("Joining session."));

			if (!Session->JoinSession(0, FName(SessionSearch->SearchResults[0].GetSessionIdStr()), SessionSearch->SearchResults[0]))
			{
				UE_LOG(LogTemp, Warning, TEXT("Join session failed"));
			}
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to find session!!"));
		// CreateEOSSession(false, false, 10);
	}
}

void UEOS_GameInstance::FindSessions()
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	// Remove the default search parameters that FOnlineSessionSearch sets up.
	SessionSearch->QuerySettings.SearchParams.Empty();
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->QuerySettings.Set(FName("IS_DEDICATED_SERVER"), true, EOnlineComparisonOp::Equals); // Seach using our Key/Value pair
	UE_LOG(LogTemp, Log, TEXT("Finding session."));
	Session->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOS_GameInstance::OnFindSessionsCompleted(bool isSuccesful)
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	FString ConnectString;
	FOnlineSessionSearchResult* SessionToJoin;
	// Створюємо масив BP-friendly структур
	TArray<FMyBlueprintSessionResult> BPResults;

	if (isSuccesful) {
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted, Num of session found: %d"), SessionSearch->SearchResults.Num());

		for (auto SessionInSearchResult : SessionSearch->SearchResults)
		{
			//Ensure the connection string is resolvable and store the info in ConnectInfo and in SessionToJoin
			if (Session->GetResolvedConnectString(SessionInSearchResult, NAME_GamePort, ConnectString))
			{
				SessionToJoin = &SessionInSearchResult;
				UE_LOG(LogTemp, Error, TEXT("Found session id: %s, opened connections: %d"), *SessionToJoin->GetSessionIdStr(), SessionToJoin->Session.NumOpenPrivateConnections);
			}

			UE_LOG(LogTemp, Error, TEXT("Session connection string: %s"), *ConnectString);

			FMyBlueprintSessionResult BPResult;
			BPResult.SessionResult = SessionInSearchResult;
			BPResult.SessionId = SessionInSearchResult.GetSessionIdStr();
			BPResult.MaxPlayers = SessionInSearchResult.Session.SessionSettings.NumPublicConnections;
			BPResult.OpenPublicConnections = SessionInSearchResult.Session.NumOpenPublicConnections;
			BPResult.OpenPrivateConnections = SessionInSearchResult.Session.NumOpenPrivateConnections;
			BPResult.ConnectionString = ConnectString;
			BPResults.Add(BPResult);
		}

		OnSessionsFound.Broadcast(BPResults);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to find session!!"));
	}
}

void UEOS_GameInstance::JoinSession(FMyBlueprintSessionResult SessionToJoin)
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	this->CurrentSession = SessionToJoin;

	if (!Session->JoinSession(0, FName(SessionToJoin.SessionResult.GetSessionIdStr()), SessionToJoin.SessionResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("Join session failed"));
	}
}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!Subsystem || !Identity) {
		UE_LOG(LogTemp, Error, TEXT("Subsystem or Identity not initialized"));
		return;
	}

	if (Result == EOnJoinSessionCompleteResult::Success) {
		FString MapName;
		FString JoinAddress;

		// Get map name from server session parameters
		Session->GetSessionSettings(SessionName)->Get(TEXT("MAP_NAME"), MapName);
		Session->GetResolvedConnectString(SessionName, JoinAddress);

		FURL ServerURL = FURL(nullptr, *JoinAddress, TRAVEL_Absolute);

		// Explicitly set map name (avoiding some weird bugs) 
		// !!! Requires setting up MAPNAME when creating session
		ServerURL.Map = MapName;

		FString OptionString = FString::Printf(TEXT("CharID=%s"), *SelectedCharacterString);
		ServerURL.AddOption(*OptionString);

		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Connection to: %s"), *ServerURL.ToString());

		FString ServerJoinError;
		auto ServerJoinStatus = GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(GetWorld()), ServerURL, ServerJoinError);

		if (ServerJoinStatus == EBrowseReturnVal::Failure) {
			UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Failed to join server. Error: %s"), *ServerJoinError);

		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Successfully joined server: %s"), *ServerURL.ToString());
		}
	}
}
