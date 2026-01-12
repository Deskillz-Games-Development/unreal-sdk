// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzHostTypes.h"
#include "DeskillzHostEvents.generated.h"

// ============================================================================
// Additional Host Event Delegates
// ============================================================================

/** Called when host registration completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostRegistered, bool, bSuccess, const FDeskillzError&, Error);

/** Called when age verification completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostVerified, bool, bSuccess, const FString&, ErrorMessage);

/** Called when experience/XP gained */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostExperienceGained, int32, XPGained, int32, NewTotal, const FString&, Source);

/** Called when host level up occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostLevelUp, int32, OldLevel, int32, NewLevel, const TArray<FString>&, UnlockedFeatures);

/** Called when hosting streak updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostStreakUpdated, int32, CurrentStreak, bool, bNewRecord);

/** Called when host rating updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostRatingUpdated, float, NewRating, float, OldRating, int32, RatingCount);

/** Called when a room is created by this host */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostRoomCreated, const FString&, RoomId, const FString&, RoomCode);

/** Called when a hosted room closes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostRoomClosed, const FString&, RoomId, double, TotalEarnings, int32, TotalPlayers);

/** Called when pending balance changes (unsettled earnings) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostPendingBalanceChanged, double, NewPending, double, Delta);

/** Called when withdrawal request is submitted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostWithdrawalRequested, const FString&, WithdrawalId, double, Amount);

/** Called when badge is about to expire */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostBadgeExpiring, const FDeskillzHostBadge&, Badge, int32, DaysRemaining);

/** Called when badge expires */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostBadgeExpired, const FDeskillzHostBadge&, Badge);

/** Called when tier evaluation is approaching */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostTierEvaluationSoon, int32, DaysRemaining, const FDeskillzHostStats&, CurrentStats);

/** Called when new notification received */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostNotificationReceived, const FDeskillzHostNotification&, Notification);

/** Called when notification settings updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostNotificationSettingsUpdated, const FDeskillzHostNotificationSettings&, Settings);

// ============================================================================
// Host Event Aggregator
// ============================================================================

/**
 * Deskillz Host Event Aggregator
 * 
 * Central hub for all Host SDK events. Provides easy Blueprint access
 * to subscribe to any host-related event in one place.
 * 
 * Usage in Blueprint:
 * - Get Deskillz Host Events node
 * - Bind to desired event (e.g., OnProfileLoaded, OnSettlementReceived)
 * 
 * Usage in C++:
 * - UDeskillzHostEvents* Events = UDeskillzHostEvents::Get(this);
 * - Events->OnTierChanged.AddDynamic(this, &MyClass::HandleTierChanged);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzHostEvents : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzHostEvents();

    /** Get the singleton host events instance */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Events", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Host Events"))
    static UDeskillzHostEvents* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Profile Events
    // ========================================================================

    /** Host profile loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Profile")
    FOnDeskillzHostProfileLoaded OnProfileLoaded;

    /** Host registered (first-time setup complete) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Profile")
    FOnDeskillzHostRegistered OnRegistered;

    /** Age verified */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Profile")
    FOnDeskillzHostVerified OnVerified;

    /** Rating updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Profile")
    FOnDeskillzHostRatingUpdated OnRatingUpdated;

    // ========================================================================
    // Tier & Level Events
    // ========================================================================

    /** Tier changed (upgrade or downgrade) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Tier")
    FOnDeskillzHostTierChanged OnTierChanged;

    /** Experience gained */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Tier")
    FOnDeskillzHostExperienceGained OnExperienceGained;

    /** Level up */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Tier")
    FOnDeskillzHostLevelUp OnLevelUp;

    /** Tier evaluation approaching */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Tier")
    FOnDeskillzHostTierEvaluationSoon OnTierEvaluationSoon;

    // ========================================================================
    // Badge Events
    // ========================================================================

    /** Badge earned */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Badges")
    FOnDeskillzHostBadgeEarned OnBadgeEarned;

    /** Badge expiring soon */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Badges")
    FOnDeskillzHostBadgeExpiring OnBadgeExpiring;

    /** Badge expired */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Badges")
    FOnDeskillzHostBadgeExpired OnBadgeExpired;

    // ========================================================================
    // Earnings Events
    // ========================================================================

    /** Earnings loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Earnings")
    FOnDeskillzHostEarningsLoaded OnEarningsLoaded;

    /** Settlement received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Earnings")
    FOnDeskillzHostSettlementReceived OnSettlementReceived;

    /** Pending balance changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Earnings")
    FOnDeskillzHostPendingBalanceChanged OnPendingBalanceChanged;

    // ========================================================================
    // Withdrawal Events
    // ========================================================================

    /** Withdrawal requested */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Withdrawal")
    FOnDeskillzHostWithdrawalRequested OnWithdrawalRequested;

    /** Withdrawal complete */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Withdrawal")
    FOnDeskillzHostWithdrawalComplete OnWithdrawalComplete;

    // ========================================================================
    // Room Events
    // ========================================================================

    /** Room created */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Rooms")
    FOnDeskillzHostRoomCreated OnRoomCreated;

    /** Room closed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Rooms")
    FOnDeskillzHostRoomClosed OnRoomClosed;

    /** Active rooms updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Rooms")
    FOnDeskillzHostActiveRoomsUpdated OnActiveRoomsUpdated;

    // ========================================================================
    // Stats Events
    // ========================================================================

    /** Stats loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Stats")
    FOnDeskillzHostStatsLoaded OnStatsLoaded;

    /** Streak updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Stats")
    FOnDeskillzHostStreakUpdated OnStreakUpdated;

    // ========================================================================
    // Notification Events
    // ========================================================================

    /** Notifications loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Notifications")
    FOnDeskillzHostNotificationsLoaded OnNotificationsLoaded;

    /** New notification received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Notifications")
    FOnDeskillzHostNotificationReceived OnNotificationReceived;

    /** Notification settings updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events|Notifications")
    FOnDeskillzHostNotificationSettingsUpdated OnNotificationSettingsUpdated;

    // ========================================================================
    // Broadcast Methods (Internal)
    // ========================================================================

    /** Broadcast profile loaded */
    void BroadcastProfileLoaded(const FDeskillzHostProfile& Profile, const FDeskillzError& Error);

    /** Broadcast tier changed */
    void BroadcastTierChanged(EDeskillzHostTier OldTier, EDeskillzHostTier NewTier, EDeskillzRoomRevenueType RevenueType);

    /** Broadcast badge earned */
    void BroadcastBadgeEarned(const FDeskillzHostBadge& Badge);

    /** Broadcast settlement received */
    void BroadcastSettlementReceived(const FString& RoomId, double Amount, const FString& Trigger);

    /** Broadcast earnings loaded */
    void BroadcastEarningsLoaded(const FDeskillzHostEarnings& Earnings, const FDeskillzError& Error);

    /** Broadcast withdrawal complete */
    void BroadcastWithdrawalComplete(const FDeskillzWithdrawalRequest& Withdrawal, const FDeskillzError& Error);

    /** Broadcast level up */
    void BroadcastLevelUp(int32 OldLevel, int32 NewLevel, const TArray<FString>& UnlockedFeatures);

    /** Broadcast notification received */
    void BroadcastNotificationReceived(const FDeskillzHostNotification& Notification);

private:
    /** Singleton instance */
    static UDeskillzHostEvents* Instance;
};

// ============================================================================
// Host Blueprint Function Library
// ============================================================================

/**
 * Blueprint function library for host convenience functions
 */
UCLASS()
class DESKILLZ_API UDeskillzHostEventLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Check if user is a verified host
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static bool IsVerifiedHost(const UObject* WorldContextObject);

    /**
     * Get current host tier name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static FString GetHostTierName(const UObject* WorldContextObject);

    /**
     * Get host share percentage
     * @param RevenueType Esports or Social
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static float GetHostSharePercent(const UObject* WorldContextObject, EDeskillzRoomRevenueType RevenueType);

    /**
     * Get available balance for withdrawal
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static double GetAvailableBalance(const UObject* WorldContextObject);

    /**
     * Get pending (unsettled) balance
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static double GetPendingBalance(const UObject* WorldContextObject);

    /**
     * Get total all-time earnings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static double GetTotalEarnings(const UObject* WorldContextObject);

    /**
     * Get current hosting streak in days
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static int32 GetHostingStreak(const UObject* WorldContextObject);

    /**
     * Get host level
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static int32 GetHostLevel(const UObject* WorldContextObject);

    /**
     * Get number of active rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static int32 GetActiveRoomCount(const UObject* WorldContextObject);

    /**
     * Check if can request withdrawal (meets minimum)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject"))
    static bool CanRequestWithdrawal(const UObject* WorldContextObject);

    /**
     * Get tier color
     * @param Tier The tier to get color for
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static FLinearColor GetTierColor(EDeskillzHostTier Tier);

    /**
     * Get tier icon string
     * @param Tier The tier to get icon for
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static FString GetTierIcon(EDeskillzHostTier Tier);

    /**
     * Get required players for tier
     * @param Tier The tier to check
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static int32 GetTierRequiredPlayers(EDeskillzHostTier Tier);

    /**
     * Calculate estimated earnings
     * @param PotAmount Average pot amount
     * @param RakePercent Rake percentage
     * @param HostSharePercent Host share percentage
     * @param RoundsPerHour Rounds per hour
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static double CalculateEstimatedEarnings(double PotAmount, float RakePercent, float HostSharePercent, int32 RoundsPerHour = 6);
};