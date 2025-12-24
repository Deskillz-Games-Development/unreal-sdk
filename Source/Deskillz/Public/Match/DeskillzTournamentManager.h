// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzTournamentManager.generated.h"

class UDeskillzSDK;

// ============================================================================
// Tournament Filter & Sort
// ============================================================================

/**
 * Tournament sort options
 */
UENUM(BlueprintType)
enum class EDeskillzTournamentSort : uint8
{
	StartTime        UMETA(DisplayName = "Start Time"),
	PrizePool        UMETA(DisplayName = "Prize Pool"),
	EntryFee         UMETA(DisplayName = "Entry Fee"),
	PlayerCount      UMETA(DisplayName = "Player Count"),
	Popularity       UMETA(DisplayName = "Popularity")
};

/**
 * Tournament filter parameters
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentFilter
{
	GENERATED_BODY()
	
	/** Filter by status (None = all) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	TArray<EDeskillzTournamentStatus> StatusFilter;
	
	/** Filter by match type (None = all) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	TArray<EDeskillzMatchType> MatchTypeFilter;
	
	/** Minimum entry fee */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	float MinEntryFee = 0.0f;
	
	/** Maximum entry fee (0 = no limit) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	float MaxEntryFee = 0.0f;
	
	/** Currency filter (empty = all) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	TArray<EDeskillzCurrency> CurrencyFilter;
	
	/** Only show tournaments user can afford */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	bool bOnlyAffordable = false;
	
	/** Only show tournaments with open slots */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	bool bOnlyOpenSlots = true;
	
	/** Search text (name/description) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	FString SearchText;
	
	/** Sort by */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	EDeskillzTournamentSort SortBy = EDeskillzTournamentSort::StartTime;
	
	/** Sort ascending */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Tournaments")
	bool bSortAscending = true;
	
	FDeskillzTournamentFilter() = default;
};

/**
 * Joined tournament status
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzJoinedTournament
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	FDeskillzTournament Tournament;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	int32 CurrentRank = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	int64 HighScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	int32 MatchesPlayed = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	int32 MatchesWon = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	double TotalEarnings = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	FDateTime JoinedAt;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	bool bCanPlay = true;
	
	FDeskillzJoinedTournament() = default;
};

/**
 * Tournament registration result
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentRegistration
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	bool bSuccess = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	FString TournamentId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	FDeskillzEntryFee EntryFeePaid;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournaments")
	FDeskillzError Error;
	
	FDeskillzTournamentRegistration() = default;
};

// ============================================================================
// Tournament Delegates
// ============================================================================

/** Called when tournaments list is updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentsUpdated, const TArray<FDeskillzTournament>&, Tournaments);

/** Called when user registers for tournament */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentRegistered, const FDeskillzTournamentRegistration&, Registration);

/** Called when user leaves tournament */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentLeft, const FString&, TournamentId);

/** Called when tournament status changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTournamentStatusChange, const FString&, TournamentId, EDeskillzTournamentStatus, NewStatus);

/** Called when joined tournaments list updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinedTournamentsUpdated, const TArray<FDeskillzJoinedTournament>&, JoinedTournaments);

// ============================================================================
// Tournament Manager
// ============================================================================

/**
 * Deskillz Tournament Manager
 * 
 * Manages tournament discovery, registration, and tracking.
 * Provides filtering, sorting, and real-time updates for tournaments.
 * 
 * Features:
 * - Tournament listing with filters
 * - Registration and withdrawal
 * - Joined tournaments tracking
 * - Real-time status updates
 * - Entry fee payment integration
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzTournamentManager : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzTournamentManager();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize tournament manager with SDK reference
	 */
	void Initialize(UDeskillzSDK* SDK);
	
	/**
	 * Cleanup tournament manager
	 */
	void Cleanup();
	
	// ========================================================================
	// Static Factory
	// ========================================================================
	
	/**
	 * Create and initialize a tournament manager instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments", meta = (WorldContext = "WorldContextObject"))
	static UDeskillzTournamentManager* CreateTournamentManager(const UObject* WorldContextObject);
	
	// ========================================================================
	// Tournament Discovery
	// ========================================================================
	
	/**
	 * Fetch available tournaments
	 * @param Filter Optional filter parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void FetchTournaments(const FDeskillzTournamentFilter& Filter = FDeskillzTournamentFilter());
	
	/**
	 * Get cached tournaments (from last fetch)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzTournament> GetCachedTournaments() const { return CachedTournaments; }
	
	/**
	 * Get filtered tournaments from cache
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzTournament> GetFilteredTournaments(const FDeskillzTournamentFilter& Filter) const;
	
	/**
	 * Get tournament by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	bool GetTournamentById(const FString& TournamentId, FDeskillzTournament& OutTournament) const;
	
	/**
	 * Fetch details for specific tournament
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void FetchTournamentDetails(const FString& TournamentId);
	
	// ========================================================================
	// Tournament Registration
	// ========================================================================
	
	/**
	 * Register for a tournament
	 * @param TournamentId Tournament to join
	 * @param PaymentCurrency Currency to pay entry fee
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void RegisterForTournament(const FString& TournamentId, EDeskillzCurrency PaymentCurrency = EDeskillzCurrency::USDT);
	
	/**
	 * Leave a tournament (before it starts)
	 * @param TournamentId Tournament to leave
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void LeaveTournament(const FString& TournamentId);
	
	/**
	 * Check if user can afford tournament entry
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	bool CanAffordEntry(const FDeskillzTournament& Tournament, EDeskillzCurrency Currency) const;
	
	/**
	 * Check if user is registered for tournament
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	bool IsRegisteredFor(const FString& TournamentId) const;
	
	// ========================================================================
	// Joined Tournaments
	// ========================================================================
	
	/**
	 * Fetch tournaments user has joined
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void FetchJoinedTournaments();
	
	/**
	 * Get cached joined tournaments
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzJoinedTournament> GetJoinedTournaments() const { return JoinedTournaments; }
	
	/**
	 * Get joined tournament by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	bool GetJoinedTournamentById(const FString& TournamentId, FDeskillzJoinedTournament& OutJoined) const;
	
	/**
	 * Get active joined tournaments (can play now)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzJoinedTournament> GetActiveTournaments() const;
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Get recommended tournaments for user
	 * Based on skill level, play history, and entry affordability
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzTournament> GetRecommendedTournaments(int32 MaxCount = 5) const;
	
	/**
	 * Get tournaments starting soon
	 * @param WithinMinutes Number of minutes to check
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	TArray<FDeskillzTournament> GetTournamentsStartingSoon(int32 WithinMinutes = 30) const;
	
	/**
	 * Format entry fee for display
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	static FString FormatEntryFee(const FDeskillzEntryFee& EntryFee);
	
	/**
	 * Format prize pool for display
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments")
	static FString FormatPrizePool(double PrizePool, EDeskillzCurrency Currency);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Tournaments list updated */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournaments|Events")
	FOnTournamentsUpdated OnTournamentsUpdated;
	
	/** User registered for tournament */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournaments|Events")
	FOnTournamentRegistered OnTournamentRegistered;
	
	/** User left tournament */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournaments|Events")
	FOnTournamentLeft OnTournamentLeft;
	
	/** Tournament status changed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournaments|Events")
	FOnTournamentStatusChange OnTournamentStatusChanged;
	
	/** Joined tournaments updated */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournaments|Events")
	FOnJoinedTournamentsUpdated OnJoinedTournamentsUpdated;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Cached tournaments */
	UPROPERTY()
	TArray<FDeskillzTournament> CachedTournaments;
	
	/** Joined tournaments */
	UPROPERTY()
	TArray<FDeskillzJoinedTournament> JoinedTournaments;
	
	/** Registered tournament IDs */
	TSet<FString> RegisteredTournamentIds;
	
	/** SDK reference */
	UPROPERTY()
	TWeakObjectPtr<UDeskillzSDK> SDKRef;
	
	/** Last fetch time */
	FDateTime LastFetchTime;
	
	/** Auto-refresh timer */
	FTimerHandle RefreshTimerHandle;
	
	/** World reference */
	TWeakObjectPtr<UWorld> WorldRef;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Apply filter to tournaments */
	TArray<FDeskillzTournament> ApplyFilter(const TArray<FDeskillzTournament>& Tournaments, const FDeskillzTournamentFilter& Filter) const;
	
	/** Sort tournaments */
	void SortTournaments(TArray<FDeskillzTournament>& Tournaments, EDeskillzTournamentSort SortBy, bool bAscending) const;
	
	/** Handle SDK tournament response */
	UFUNCTION()
	void OnTournamentsReceived(const TArray<FDeskillzTournament>& Tournaments, const FDeskillzError& Error);
	
	/** Auto-refresh tournaments */
	void AutoRefresh();
	
	/** Update registered tournament IDs from joined list */
	void UpdateRegisteredIds();
};
