// Copyright Deskillz Games. All Rights Reserved.
// DeskillzDeepLinkHandler.cpp - Implementation of deep link handler for centralized lobby

#include "Lobby/DeskillzDeepLinkHandler.h"
#include "Misc/DateTime.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

// Singleton instance
UDeskillzDeepLinkHandler* UDeskillzDeepLinkHandler::Instance = nullptr;

UDeskillzDeepLinkHandler::UDeskillzDeepLinkHandler()
{
	// Constructor
}

UDeskillzDeepLinkHandler* UDeskillzDeepLinkHandler::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UDeskillzDeepLinkHandler>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}

void UDeskillzDeepLinkHandler::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Initializing..."));
	
	// Register for platform deep link events
	RegisterPlatformHandler();
	
	bIsInitialized = true;
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Initialized successfully"));
	
	// Check for launch URL that started the app
	FString LaunchURL;
#if PLATFORM_IOS || PLATFORM_ANDROID
	// Platform-specific launch URL retrieval would go here
	// For now, check command line
	FParse::Value(FCommandLine::Get(), TEXT("-deeplink="), LaunchURL);
#endif
	
	if (!LaunchURL.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Found launch URL: %s"), *LaunchURL);
		HandleDeepLink(LaunchURL);
	}
}

void UDeskillzDeepLinkHandler::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Shutting down..."));
	
	UnregisterPlatformHandler();
	
	bIsInitialized = false;
}

bool UDeskillzDeepLinkHandler::HandleDeepLink(const FString& URL)
{
	if (URL.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzDeepLinkHandler] Empty URL received"));
		return false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Handling deep link: %s"), *URL);
	
	// =====================================================
	// STEP 1: Check for navigation deep links FIRST
	// These are simpler links like deskillz://tournaments
	// =====================================================
	EDeskillzNavigationAction NavAction;
	FString NavTargetId;
	if (ParseNavigationLink(URL, NavAction, NavTargetId))
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Navigation deep link detected - Action: %d, Target: %s"), 
			static_cast<int32>(NavAction), *NavTargetId);
		
		// Broadcast navigation event
		OnNavigationReceived.Broadcast(NavAction, NavTargetId);
		return true;
	}
	
	// =====================================================
	// STEP 2: Check if this is a match launch deep link
	// =====================================================
	if (!IsLaunchDeepLink(URL))
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Not a recognized deep link, ignoring"));
		return false;
	}
	
	// Parse the URL into launch data
	FDeskillzMatchLaunchData LaunchData = ParseLaunchURL(URL);
	
	if (!LaunchData.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzDeepLinkHandler] Failed to parse launch URL"));
		return false;
	}
	
	// If not initialized yet, store as pending
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Storing as pending launch (not yet initialized)"));
		PendingLaunchData = LaunchData;
		return true;
	}
	
	// Process the launch data
	ProcessLaunchData(LaunchData);
	
	return true;
}

// =============================================================================
// NAVIGATION DEEP LINK PARSING
// =============================================================================

bool UDeskillzDeepLinkHandler::IsNavigationDeepLink(const FString& URL) const
{
	FString LowerURL = URL.ToLower();
	
	// Navigation links don't have matchId parameter
	if (LowerURL.Contains(TEXT("matchid=")))
	{
		return false;
	}
	
	// Check for navigation paths
	return LowerURL.Contains(TEXT("://tournaments")) ||
		   LowerURL.Contains(TEXT("://tournament")) ||
		   LowerURL.Contains(TEXT("://wallet")) ||
		   LowerURL.Contains(TEXT("://profile")) ||
		   LowerURL.Contains(TEXT("://settings")) ||
		   (LowerURL.Contains(TEXT("://game")) && !LowerURL.Contains(TEXT("matchid=")));
}

bool UDeskillzDeepLinkHandler::ParseNavigationLink(const FString& URL, EDeskillzNavigationAction& OutAction, FString& OutTargetId) const
{
	OutAction = EDeskillzNavigationAction::None;
	OutTargetId = TEXT("");
	
	if (URL.IsEmpty())
	{
		return false;
	}
	
	// First check if it's a navigation link at all
	if (!IsNavigationDeepLink(URL))
	{
		return false;
	}
	
	// Find the path after ://
	int32 SchemeEnd = URL.Find(TEXT("://"));
	if (SchemeEnd == INDEX_NONE)
	{
		return false;
	}
	
	FString Remainder = URL.Mid(SchemeEnd + 3);
	
	// Split path from query
	FString Path;
	FString Query;
	int32 QueryStart = Remainder.Find(TEXT("?"));
	if (QueryStart != INDEX_NONE)
	{
		Path = Remainder.Left(QueryStart);
		Query = Remainder.Mid(QueryStart + 1);
	}
	else
	{
		Path = Remainder;
	}
	
	// Remove trailing slashes and convert to lowercase
	Path.TrimEndInline();
	while (Path.EndsWith(TEXT("/")))
	{
		Path = Path.LeftChop(1);
	}
	Path = Path.ToLower();
	
	// Parse based on path
	if (Path == TEXT("tournaments") || Path == TEXT("tournament"))
	{
		OutAction = EDeskillzNavigationAction::Tournaments;
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Tournaments"));
		return true;
	}
	else if (Path == TEXT("wallet"))
	{
		OutAction = EDeskillzNavigationAction::Wallet;
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Wallet"));
		return true;
	}
	else if (Path == TEXT("profile"))
	{
		OutAction = EDeskillzNavigationAction::Profile;
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Profile"));
		return true;
	}
	else if (Path == TEXT("settings"))
	{
		OutAction = EDeskillzNavigationAction::Settings;
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Settings"));
		return true;
	}
	else if (Path == TEXT("game") || Path == TEXT("games"))
	{
		// Extract game ID from query string
		TMap<FString, FString> Params = ParseQueryParameters(URL);
		
		if (Params.Contains(TEXT("id")))
		{
			OutTargetId = URLDecode(Params[TEXT("id")]);
			OutAction = EDeskillzNavigationAction::Game;
			UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Game (ID: %s)"), *OutTargetId);
			return true;
		}
		else if (Params.Contains(TEXT("gameId")))
		{
			OutTargetId = URLDecode(Params[TEXT("gameId")]);
			OutAction = EDeskillzNavigationAction::Game;
			UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Game (ID: %s)"), *OutTargetId);
			return true;
		}
		else if (Params.Contains(TEXT("game_id")))
		{
			OutTargetId = URLDecode(Params[TEXT("game_id")]);
			OutAction = EDeskillzNavigationAction::Game;
			UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Game (ID: %s)"), *OutTargetId);
			return true;
		}
		
		// Game path without ID - still valid, just show game list
		OutAction = EDeskillzNavigationAction::Game;
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed navigation: Game (no specific ID)"));
		return true;
	}
	
	return false;
}

// =============================================================================
// MATCH LAUNCH DEEP LINK PARSING
// =============================================================================

FDeskillzMatchLaunchData UDeskillzDeepLinkHandler::ParseLaunchURL(const FString& URL) const
{
	FDeskillzMatchLaunchData LaunchData;
	LaunchData.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	
	// Parse query parameters
	TMap<FString, FString> Params = ParseQueryParameters(URL);
	
	// Required parameters
	if (Params.Contains(TEXT("matchId")))
	{
		LaunchData.MatchId = URLDecode(Params[TEXT("matchId")]);
	}
	
	if (Params.Contains(TEXT("token")))
	{
		LaunchData.PlayerToken = URLDecode(Params[TEXT("token")]);
	}
	
	// Optional parameters
	if (Params.Contains(TEXT("tournamentId")))
	{
		LaunchData.TournamentId = URLDecode(Params[TEXT("tournamentId")]);
	}
	
	if (Params.Contains(TEXT("matchType")))
	{
		FString MatchTypeStr = Params[TEXT("matchType")].ToLower();
		if (MatchTypeStr == TEXT("sync") || MatchTypeStr == TEXT("synchronous"))
		{
			LaunchData.MatchType = EDeskillzMatchType::Synchronous;
		}
		else
		{
			LaunchData.MatchType = EDeskillzMatchType::Asynchronous;
		}
	}
	
	if (Params.Contains(TEXT("duration")))
	{
		LaunchData.DurationSeconds = FCString::Atoi(*Params[TEXT("duration")]);
	}
	
	if (Params.Contains(TEXT("seed")))
	{
		LaunchData.RandomSeed = FCString::Atoi64(*Params[TEXT("seed")]);
	}
	
	if (Params.Contains(TEXT("entryFee")))
	{
		LaunchData.EntryFee = FCString::Atod(*Params[TEXT("entryFee")]);
	}
	
	if (Params.Contains(TEXT("currency")))
	{
		FString CurrencyStr = Params[TEXT("currency")].ToUpper();
		if (CurrencyStr == TEXT("BTC")) LaunchData.Currency = EDeskillzCurrency::BTC;
		else if (CurrencyStr == TEXT("ETH")) LaunchData.Currency = EDeskillzCurrency::ETH;
		else if (CurrencyStr == TEXT("SOL")) LaunchData.Currency = EDeskillzCurrency::SOL;
		else if (CurrencyStr == TEXT("XRP")) LaunchData.Currency = EDeskillzCurrency::XRP;
		else if (CurrencyStr == TEXT("BNB")) LaunchData.Currency = EDeskillzCurrency::BNB;
		else if (CurrencyStr == TEXT("USDC")) LaunchData.Currency = EDeskillzCurrency::USDC;
		else LaunchData.Currency = EDeskillzCurrency::USDT;
	}
	
	if (Params.Contains(TEXT("prizePool")))
	{
		LaunchData.PrizePool = FCString::Atod(*Params[TEXT("prizePool")]);
	}
	
	if (Params.Contains(TEXT("rounds")))
	{
		LaunchData.Rounds = FCString::Atoi(*Params[TEXT("rounds")]);
	}
	
	if (Params.Contains(TEXT("roomCode")))
	{
		LaunchData.RoomCode = URLDecode(Params[TEXT("roomCode")]);
		LaunchData.bIsPrivateRoom = true;
	}
	
	if (Params.Contains(TEXT("scoreType")))
	{
		LaunchData.ScoreType = URLDecode(Params[TEXT("scoreType")]);
	}
	
	// Parse opponent info if provided
	if (Params.Contains(TEXT("opponentId")))
	{
		LaunchData.Opponent.PlayerId = URLDecode(Params[TEXT("opponentId")]);
	}
	if (Params.Contains(TEXT("opponentName")))
	{
		LaunchData.Opponent.Username = URLDecode(Params[TEXT("opponentName")]);
	}
	if (Params.Contains(TEXT("opponentAvatar")))
	{
		LaunchData.Opponent.AvatarUrl = URLDecode(Params[TEXT("opponentAvatar")]);
	}
	if (Params.Contains(TEXT("opponentRating")))
	{
		LaunchData.Opponent.Rating = FCString::Atoi(*Params[TEXT("opponentRating")]);
	}
	
	// Parse any custom parameters (prefixed with "custom_")
	for (const auto& Pair : Params)
	{
		if (Pair.Key.StartsWith(TEXT("custom_")))
		{
			FString CustomKey = Pair.Key.RightChop(7); // Remove "custom_" prefix
			LaunchData.CustomParams.Add(CustomKey, URLDecode(Pair.Value));
		}
	}
	
	// Validate required fields
	LaunchData.bIsValid = !LaunchData.MatchId.IsEmpty() && !LaunchData.PlayerToken.IsEmpty();
	
	if (LaunchData.bIsValid)
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Parsed launch data - MatchId: %s, Duration: %d, Type: %s"),
			*LaunchData.MatchId,
			LaunchData.DurationSeconds,
			LaunchData.IsSynchronous() ? TEXT("Sync") : TEXT("Async"));
	}
	
	return LaunchData;
}

bool UDeskillzDeepLinkHandler::ValidateLaunchData(const FDeskillzMatchLaunchData& LaunchData, FString& OutError) const
{
	// Check basic validity
	if (!LaunchData.bIsValid)
	{
		OutError = TEXT("Invalid launch data");
		return false;
	}
	
	// Check required fields
	if (LaunchData.MatchId.IsEmpty())
	{
		OutError = TEXT("Missing match ID");
		return false;
	}
	
	if (LaunchData.PlayerToken.IsEmpty())
	{
		OutError = TEXT("Missing player token");
		return false;
	}
	
	// Check token expiration
	int64 CurrentTime = FDateTime::UtcNow().ToUnixTimestamp();
	int64 TokenAge = CurrentTime - LaunchData.Timestamp;
	
	if (TokenAge > TokenExpirationSeconds)
	{
		OutError = FString::Printf(TEXT("Launch token expired (age: %lld seconds)"), TokenAge);
		return false;
	}
	
	// Check duration is reasonable
	if (LaunchData.DurationSeconds < 10 || LaunchData.DurationSeconds > 3600)
	{
		OutError = FString::Printf(TEXT("Invalid match duration: %d seconds"), LaunchData.DurationSeconds);
		return false;
	}
	
	// All checks passed
	return true;
}

bool UDeskillzDeepLinkHandler::GetPendingLaunch(FDeskillzMatchLaunchData& OutLaunchData)
{
	if (!PendingLaunchData.bIsValid)
	{
		return false;
	}
	
	OutLaunchData = PendingLaunchData;
	return true;
}

void UDeskillzDeepLinkHandler::ClearPendingLaunch()
{
	PendingLaunchData = FDeskillzMatchLaunchData();
}

void UDeskillzDeepLinkHandler::ProcessPendingLaunch()
{
	if (!PendingLaunchData.bIsValid)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Processing pending launch"));
	
	FDeskillzMatchLaunchData LaunchData = PendingLaunchData;
	ClearPendingLaunch();
	
	ProcessLaunchData(LaunchData);
}

void UDeskillzDeepLinkHandler::ProcessLaunchData(const FDeskillzMatchLaunchData& LaunchData)
{
	// Store as current
	CurrentLaunchData = LaunchData;
	
	// Broadcast that we received a launch
	OnMatchLaunchReceived.Broadcast(LaunchData);
	
	// Auto-validate if enabled
	if (bAutoValidateLaunch)
	{
		FString ValidationError;
		if (ValidateLaunchData(LaunchData, ValidationError))
		{
			UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Launch validated, match ready"));
			OnMatchReady.Broadcast(LaunchData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[DeskillzDeepLinkHandler] Launch validation failed: %s"), *ValidationError);
			OnValidationFailed.Broadcast(ValidationError, LaunchData);
		}
	}
	else
	{
		// Caller will validate manually
		UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Launch received, awaiting manual validation"));
	}
}

bool UDeskillzDeepLinkHandler::IsLaunchDeepLink(const FString& URL) const
{
	// Check for custom scheme launch paths
	if (URL.StartsWith(URLScheme + TEXT("://launch")) ||
		URL.StartsWith(URLScheme + TEXT("://match/start")))
	{
		return true;
	}
	
	// Check for universal link
	if (URL.Contains(TEXT("deskillz.games")) && 
		(URL.Contains(TEXT("/launch")) || URL.Contains(TEXT("/start"))))
	{
		return true;
	}
	
	// Check for required match parameters
	if (URL.Contains(TEXT("matchId=")) && URL.Contains(TEXT("token=")))
	{
		return true;
	}
	
	return false;
}

TMap<FString, FString> UDeskillzDeepLinkHandler::ParseQueryParameters(const FString& URL) const
{
	TMap<FString, FString> Params;
	
	// Find query string start
	int32 QueryStart = URL.Find(TEXT("?"));
	if (QueryStart == INDEX_NONE)
	{
		return Params;
	}
	
	FString QueryString = URL.RightChop(QueryStart + 1);
	
	// Remove fragment if present
	int32 FragmentStart = QueryString.Find(TEXT("#"));
	if (FragmentStart != INDEX_NONE)
	{
		QueryString = QueryString.Left(FragmentStart);
	}
	
	// Parse parameters
	TArray<FString> Pairs;
	QueryString.ParseIntoArray(Pairs, TEXT("&"), true);
	
	for (const FString& Pair : Pairs)
	{
		int32 EqualsPos = Pair.Find(TEXT("="));
		if (EqualsPos != INDEX_NONE)
		{
			FString Key = Pair.Left(EqualsPos);
			FString Value = Pair.RightChop(EqualsPos + 1);
			Params.Add(Key, Value);
		}
	}
	
	return Params;
}

FString UDeskillzDeepLinkHandler::URLDecode(const FString& EncodedString) const
{
	FString DecodedString = EncodedString;
	
	// Replace + with space
	DecodedString = DecodedString.Replace(TEXT("+"), TEXT(" "));
	
	// Decode percent-encoded characters
	FString Result;
	for (int32 i = 0; i < DecodedString.Len(); i++)
	{
		if (DecodedString[i] == '%' && i + 2 < DecodedString.Len())
		{
			FString HexStr = DecodedString.Mid(i + 1, 2);
			int32 CharCode = 0;
			if (FParse::HexNumber(*HexStr, (uint32&)CharCode))
			{
				Result.AppendChar((TCHAR)CharCode);
				i += 2;
				continue;
			}
		}
		Result.AppendChar(DecodedString[i]);
	}
	
	return Result;
}

void UDeskillzDeepLinkHandler::RegisterPlatformHandler()
{
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Registering platform handler"));
	
#if PLATFORM_IOS
	// iOS: Register for URL scheme handling
	// This would integrate with IOSAppDelegate
#endif
	
#if PLATFORM_ANDROID
	// Android: Register for Intent handling
	// This would integrate with AndroidJNI
#endif
}

void UDeskillzDeepLinkHandler::UnregisterPlatformHandler()
{
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Unregistering platform handler"));
	
#if PLATFORM_IOS
	// iOS cleanup
#endif
	
#if PLATFORM_ANDROID
	// Android cleanup
#endif
}

void UDeskillzDeepLinkHandler::OnPlatformDeepLink(const FString& URL)
{
	UE_LOG(LogTemp, Log, TEXT("[DeskillzDeepLinkHandler] Platform deep link received: %s"), *URL);
	HandleDeepLink(URL);
}