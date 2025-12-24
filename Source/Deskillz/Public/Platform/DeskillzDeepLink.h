// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzDeepLink.generated.h"

/**
 * Deep link action type
 */
UENUM(BlueprintType)
enum class EDeskillzDeepLinkAction : uint8
{
	/** No specific action */
	None,
	
	/** Open tournament */
	OpenTournament,
	
	/** Open match */
	OpenMatch,
	
	/** Open profile */
	OpenProfile,
	
	/** Open game */
	OpenGame,
	
	/** Open wallet */
	OpenWallet,
	
	/** Apply promo code */
	ApplyPromo,
	
	/** Friend invite */
	FriendInvite,
	
	/** Custom action */
	Custom
};

/**
 * Parsed deep link data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzDeepLinkData
{
	GENERATED_BODY()
	
	/** Original URL */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	FString OriginalURL;
	
	/** Scheme (e.g., "deskillz") */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	FString Scheme;
	
	/** Host/path (e.g., "tournament") */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	FString Host;
	
	/** Path components */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	TArray<FString> PathComponents;
	
	/** Query parameters */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	TMap<FString, FString> Parameters;
	
	/** Detected action */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	EDeskillzDeepLinkAction Action = EDeskillzDeepLinkAction::None;
	
	/** Target ID (tournament ID, match ID, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	FString TargetId;
	
	/** Is valid deep link */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	bool bIsValid = false;
	
	/** Timestamp when received */
	UPROPERTY(BlueprintReadOnly, Category = "DeepLink")
	int64 Timestamp = 0;
};

/** Deep link delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeepLinkReceived, const FDeskillzDeepLinkData&, DeepLink);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeepLinkAction, EDeskillzDeepLinkAction, Action, const FString&, TargetId);

/**
 * Deskillz Deep Link Handler
 * 
 * Handle deep links for:
 * - App launches from external links
 * - Universal/App Links
 * - Custom URL schemes
 * - Referral and invite links
 * 
 * URL Formats:
 *   deskillz://tournament/123
 *   deskillz://match/456
 *   deskillz://profile/user_789
 *   deskillz://promo?code=BONUS50
 *   https://www.deskillz.games/t/123 (Universal Link)
 * 
 * Usage:
 *   UDeskillzDeepLink* DeepLink = UDeskillzDeepLink::Get();
 *   DeepLink->OnDeepLinkReceived.AddDynamic(this, &AMyActor::HandleDeepLink);
 *   
 *   // Generate share link
 *   FString ShareURL = DeepLink->GenerateTournamentLink("tournament_123");
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzDeepLink : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzDeepLink();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Deep Link instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|DeepLink", meta = (DisplayName = "Get Deskillz Deep Link"))
	static UDeskillzDeepLink* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize deep link handling
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	void Initialize();
	
	/**
	 * Set custom URL scheme
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	void SetURLScheme(const FString& Scheme);
	
	/**
	 * Set universal link domain
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	void SetUniversalLinkDomain(const FString& Domain);
	
	// ========================================================================
	// Deep Link Handling
	// ========================================================================
	
	/**
	 * Handle incoming deep link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	bool HandleDeepLink(const FString& URL);
	
	/**
	 * Parse deep link URL
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FDeskillzDeepLinkData ParseDeepLink(const FString& URL) const;
	
	/**
	 * Get pending deep link (received before initialization)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	bool GetPendingDeepLink(FDeskillzDeepLinkData& OutDeepLink);
	
	/**
	 * Clear pending deep link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	void ClearPendingDeepLink();
	
	/**
	 * Check if has pending deep link
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|DeepLink")
	bool HasPendingDeepLink() const { return PendingDeepLink.bIsValid; }
	
	// ========================================================================
	// Link Generation
	// ========================================================================
	
	/**
	 * Generate tournament share link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GenerateTournamentLink(const FString& TournamentId) const;
	
	/**
	 * Generate match share link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GenerateMatchLink(const FString& MatchId) const;
	
	/**
	 * Generate profile share link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GenerateProfileLink(const FString& UserId) const;
	
	/**
	 * Generate referral link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GenerateReferralLink(const FString& ReferralCode) const;
	
	/**
	 * Generate promo link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GeneratePromoLink(const FString& PromoCode) const;
	
	/**
	 * Generate custom link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|DeepLink")
	FString GenerateCustomLink(const FString& Path, const TMap<FString, FString>& Parameters) const;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when deep link is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|DeepLink")
	FOnDeepLinkReceived OnDeepLinkReceived;
	
	/** Called when specific action is detected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|DeepLink")
	FOnDeepLinkAction OnDeepLinkAction;
	
protected:
	/** URL scheme (e.g., "deskillz") */
	UPROPERTY()
	FString URLScheme = TEXT("deskillz");
	
	/** Universal link domain */
	UPROPERTY()
	FString UniversalLinkDomain = TEXT("www.deskillz.games");
	
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Pending deep link */
	UPROPERTY()
	FDeskillzDeepLinkData PendingDeepLink;
	
	/** Last processed deep link */
	UPROPERTY()
	FDeskillzDeepLinkData LastDeepLink;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Detect action from parsed data */
	EDeskillzDeepLinkAction DetectAction(const FString& Host, const TArray<FString>& PathComponents) const;
	
	/** Parse URL query string */
	TMap<FString, FString> ParseQueryString(const FString& QueryString) const;
	
	/** Build URL with parameters */
	FString BuildURL(const FString& Path, const TMap<FString, FString>& Parameters) const;
	
	/** Register for platform deep link events */
	void RegisterPlatformHandler();
	
	/** Handle platform callback */
	void OnPlatformDeepLink(const FString& URL);
};
