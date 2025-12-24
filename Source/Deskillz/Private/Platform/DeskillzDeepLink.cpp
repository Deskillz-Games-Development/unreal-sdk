// Copyright Deskillz Games. All Rights Reserved.

#include "Platform/DeskillzDeepLink.h"
#include "Deskillz.h"
#include "Misc/CommandLine.h"
#include "GenericPlatform/GenericPlatformHttp.h"

// Static singleton
static UDeskillzDeepLink* GDeepLink = nullptr;

UDeskillzDeepLink::UDeskillzDeepLink()
{
}

UDeskillzDeepLink* UDeskillzDeepLink::Get()
{
	if (!GDeepLink)
	{
		GDeepLink = NewObject<UDeskillzDeepLink>();
		GDeepLink->AddToRoot();
	}
	return GDeepLink;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzDeepLink::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}
	
	// Register platform handler
	RegisterPlatformHandler();
	
	// Check for launch URL from command line
	FString LaunchURL;
	if (FParse::Value(FCommandLine::Get(), TEXT("-url="), LaunchURL))
	{
		HandleDeepLink(LaunchURL);
	}
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("DeepLink initialized - Scheme: %s, Domain: %s"), 
		*URLScheme, *UniversalLinkDomain);
}

void UDeskillzDeepLink::SetURLScheme(const FString& Scheme)
{
	URLScheme = Scheme;
}

void UDeskillzDeepLink::SetUniversalLinkDomain(const FString& Domain)
{
	UniversalLinkDomain = Domain;
}

// ============================================================================
// Deep Link Handling
// ============================================================================

bool UDeskillzDeepLink::HandleDeepLink(const FString& URL)
{
	if (URL.IsEmpty())
	{
		return false;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Handling deep link: %s"), *URL);
	
	// Parse the deep link
	FDeskillzDeepLinkData ParsedData = ParseDeepLink(URL);
	
	if (!ParsedData.bIsValid)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Invalid deep link: %s"), *URL);
		return false;
	}
	
	// Store as last/pending
	LastDeepLink = ParsedData;
	
	if (!bIsInitialized)
	{
		// Store for later processing
		PendingDeepLink = ParsedData;
		UE_LOG(LogDeskillz, Log, TEXT("Deep link stored as pending (not initialized yet)"));
		return true;
	}
	
	// Broadcast events
	OnDeepLinkReceived.Broadcast(ParsedData);
	
	if (ParsedData.Action != EDeskillzDeepLinkAction::None)
	{
		OnDeepLinkAction.Broadcast(ParsedData.Action, ParsedData.TargetId);
	}
	
	return true;
}

FDeskillzDeepLinkData UDeskillzDeepLink::ParseDeepLink(const FString& URL) const
{
	FDeskillzDeepLinkData Data;
	Data.OriginalURL = URL;
	Data.Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	
	if (URL.IsEmpty())
	{
		return Data;
	}
	
	// Parse scheme
	int32 SchemeEnd = URL.Find(TEXT("://"));
	if (SchemeEnd == INDEX_NONE)
	{
		return Data;
	}
	
	Data.Scheme = URL.Left(SchemeEnd).ToLower();
	
	// Check if valid scheme
	bool bIsCustomScheme = Data.Scheme == URLScheme.ToLower();
	bool bIsHttpScheme = Data.Scheme == TEXT("http") || Data.Scheme == TEXT("https");
	
	if (!bIsCustomScheme && !bIsHttpScheme)
	{
		return Data;
	}
	
	// Extract remainder after scheme
	FString Remainder = URL.Mid(SchemeEnd + 3);
	
	// Split host/path from query
	FString HostPath;
	FString QueryString;
	
	int32 QueryStart = Remainder.Find(TEXT("?"));
	if (QueryStart != INDEX_NONE)
	{
		HostPath = Remainder.Left(QueryStart);
		QueryString = Remainder.Mid(QueryStart + 1);
	}
	else
	{
		HostPath = Remainder;
	}
	
	// Remove trailing slash
	HostPath.TrimEndInline();
	if (HostPath.EndsWith(TEXT("/")))
	{
		HostPath = HostPath.LeftChop(1);
	}
	
	// Parse host and path
	TArray<FString> Parts;
	HostPath.ParseIntoArray(Parts, TEXT("/"), true);
	
	if (Parts.Num() > 0)
	{
		// For HTTP URLs, first part is domain
		if (bIsHttpScheme)
		{
			FString Domain = Parts[0];
			
			// Check if it's our universal link domain
			if (!Domain.Contains(UniversalLinkDomain))
			{
				// Not our domain
				return Data;
			}
			
			// Remove domain from parts
			Parts.RemoveAt(0);
		}
		
		// First path component is the host/action
		if (Parts.Num() > 0)
		{
			Data.Host = Parts[0].ToLower();
			Parts.RemoveAt(0);
		}
		
		// Remaining are path components
		Data.PathComponents = Parts;
	}
	
	// Parse query parameters
	Data.Parameters = ParseQueryString(QueryString);
	
	// Detect action
	Data.Action = DetectAction(Data.Host, Data.PathComponents);
	
	// Extract target ID based on action
	switch (Data.Action)
	{
		case EDeskillzDeepLinkAction::OpenTournament:
		case EDeskillzDeepLinkAction::OpenMatch:
		case EDeskillzDeepLinkAction::OpenProfile:
		case EDeskillzDeepLinkAction::OpenGame:
			if (Data.PathComponents.Num() > 0)
			{
				Data.TargetId = Data.PathComponents[0];
			}
			break;
			
		case EDeskillzDeepLinkAction::ApplyPromo:
			if (const FString* Code = Data.Parameters.Find(TEXT("code")))
			{
				Data.TargetId = *Code;
			}
			break;
			
		case EDeskillzDeepLinkAction::FriendInvite:
			if (const FString* RefCode = Data.Parameters.Find(TEXT("ref")))
			{
				Data.TargetId = *RefCode;
			}
			break;
			
		default:
			break;
	}
	
	Data.bIsValid = !Data.Host.IsEmpty();
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Parsed deep link - Action: %d, Target: %s"), 
		static_cast<int32>(Data.Action), *Data.TargetId);
	
	return Data;
}

bool UDeskillzDeepLink::GetPendingDeepLink(FDeskillzDeepLinkData& OutDeepLink)
{
	if (!PendingDeepLink.bIsValid)
	{
		return false;
	}
	
	OutDeepLink = PendingDeepLink;
	return true;
}

void UDeskillzDeepLink::ClearPendingDeepLink()
{
	PendingDeepLink = FDeskillzDeepLinkData();
}

// ============================================================================
// Link Generation
// ============================================================================

FString UDeskillzDeepLink::GenerateTournamentLink(const FString& TournamentId) const
{
	return FString::Printf(TEXT("https://%s/tournament/%s"), *UniversalLinkDomain, *TournamentId);
}

FString UDeskillzDeepLink::GenerateMatchLink(const FString& MatchId) const
{
	return FString::Printf(TEXT("https://%s/match/%s"), *UniversalLinkDomain, *MatchId);
}

FString UDeskillzDeepLink::GenerateProfileLink(const FString& UserId) const
{
	return FString::Printf(TEXT("https://%s/profile/%s"), *UniversalLinkDomain, *UserId);
}

FString UDeskillzDeepLink::GenerateReferralLink(const FString& ReferralCode) const
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("ref"), ReferralCode);
	return BuildURL(TEXT("invite"), Params);
}

FString UDeskillzDeepLink::GeneratePromoLink(const FString& PromoCode) const
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("code"), PromoCode);
	return BuildURL(TEXT("promo"), Params);
}

FString UDeskillzDeepLink::GenerateCustomLink(const FString& Path, const TMap<FString, FString>& Parameters) const
{
	return BuildURL(Path, Parameters);
}

// ============================================================================
// Internal Methods
// ============================================================================

EDeskillzDeepLinkAction UDeskillzDeepLink::DetectAction(const FString& Host, const TArray<FString>& PathComponents) const
{
	FString LowerHost = Host.ToLower();
	
	// Tournament variants
	if (LowerHost == TEXT("tournament") || LowerHost == TEXT("t") || LowerHost == TEXT("tournaments"))
	{
		return EDeskillzDeepLinkAction::OpenTournament;
	}
	
	// Match variants
	if (LowerHost == TEXT("match") || LowerHost == TEXT("m") || LowerHost == TEXT("matches"))
	{
		return EDeskillzDeepLinkAction::OpenMatch;
	}
	
	// Profile variants
	if (LowerHost == TEXT("profile") || LowerHost == TEXT("user") || LowerHost == TEXT("u") || LowerHost == TEXT("p"))
	{
		return EDeskillzDeepLinkAction::OpenProfile;
	}
	
	// Game variants
	if (LowerHost == TEXT("game") || LowerHost == TEXT("g") || LowerHost == TEXT("games"))
	{
		return EDeskillzDeepLinkAction::OpenGame;
	}
	
	// Wallet
	if (LowerHost == TEXT("wallet") || LowerHost == TEXT("w"))
	{
		return EDeskillzDeepLinkAction::OpenWallet;
	}
	
	// Promo
	if (LowerHost == TEXT("promo") || LowerHost == TEXT("coupon") || LowerHost == TEXT("code"))
	{
		return EDeskillzDeepLinkAction::ApplyPromo;
	}
	
	// Invite/referral
	if (LowerHost == TEXT("invite") || LowerHost == TEXT("ref") || LowerHost == TEXT("referral"))
	{
		return EDeskillzDeepLinkAction::FriendInvite;
	}
	
	return EDeskillzDeepLinkAction::Custom;
}

TMap<FString, FString> UDeskillzDeepLink::ParseQueryString(const FString& QueryString) const
{
	TMap<FString, FString> Parameters;
	
	if (QueryString.IsEmpty())
	{
		return Parameters;
	}
	
	TArray<FString> Pairs;
	QueryString.ParseIntoArray(Pairs, TEXT("&"), true);
	
	for (const FString& Pair : Pairs)
	{
		FString Key, Value;
		if (Pair.Split(TEXT("="), &Key, &Value))
		{
			// URL decode
			Key = FGenericPlatformHttp::UrlDecode(Key);
			Value = FGenericPlatformHttp::UrlDecode(Value);
			
			Parameters.Add(Key, Value);
		}
	}
	
	return Parameters;
}

FString UDeskillzDeepLink::BuildURL(const FString& Path, const TMap<FString, FString>& Parameters) const
{
	FString URL = FString::Printf(TEXT("https://%s/%s"), *UniversalLinkDomain, *Path);
	
	if (Parameters.Num() > 0)
	{
		URL += TEXT("?");
		
		bool bFirst = true;
		for (const auto& Pair : Parameters)
		{
			if (!bFirst)
			{
				URL += TEXT("&");
			}
			bFirst = false;
			
			URL += FGenericPlatformHttp::UrlEncode(Pair.Key);
			URL += TEXT("=");
			URL += FGenericPlatformHttp::UrlEncode(Pair.Value);
		}
	}
	
	return URL;
}

void UDeskillzDeepLink::RegisterPlatformHandler()
{
	// Platform-specific deep link registration would go here
	// This typically requires native iOS/Android code
	
#if PLATFORM_IOS
	// iOS uses URL schemes and Universal Links
	// Registration happens in Info.plist and Apple Developer portal
#elif PLATFORM_ANDROID
	// Android uses Intent filters
	// Registration happens in AndroidManifest.xml
#endif
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Platform deep link handler registered"));
}

void UDeskillzDeepLink::OnPlatformDeepLink(const FString& URL)
{
	HandleDeepLink(URL);
}
