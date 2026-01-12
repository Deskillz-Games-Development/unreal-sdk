// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzHostTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzHostManager.generated.h"

class UDeskillzSDK;

/**
 * Deskillz Host Manager
 * 
 * Manages host profiles, tiers, earnings, and revenue sharing for room hosts.
 * Provides access to the 6-tier host system with revenue percentages ranging
 * from 15% (Bronze) to 28% (Elite).
 * 
 * Features:
 * - Host profile management
 * - Dual tier tracking (Esports & Social)
 * - Earnings and settlements
 * - Badge system with bonus percentages
 * - Withdrawal processing
 * 
 * Usage:
 * 1. Get via GetGameInstance()->GetSubsystem<UDeskillzHostManager>()
 * 2. Call LoadProfile() to fetch host data
 * 3. Subscribe to events for real-time updates
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzHostManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDeskillzHostManager();

    // ========================================================================
    // Subsystem Lifecycle
    // ========================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // ========================================================================
    // Static Accessors
    // ========================================================================

    /**
     * Get the Host Manager instance
     * @param WorldContextObject Any UObject to get world context from
     * @return The Host Manager instance or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Host Manager"))
    static UDeskillzHostManager* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Profile Management
    // ========================================================================

    /**
     * Load the current user's host profile
     * Results returned via OnProfileLoaded delegate
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void LoadProfile();

    /**
     * Get the cached host profile
     * @return Current host profile (may be empty if not loaded)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    FDeskillzHostProfile GetProfile() const { return CurrentProfile; }

    /**
     * Check if host profile is loaded
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    bool IsProfileLoaded() const { return bProfileLoaded; }

    /**
     * Check if user is a verified host (18+)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    bool IsVerifiedHost() const { return CurrentProfile.bIsVerified; }

    /**
     * Register as a host (first-time setup)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void RegisterAsHost();

    /**
     * Verify host age (18+)
     * @param VerificationData Age verification data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void VerifyAge(const FString& VerificationData);

    // ========================================================================
    // Tier System
    // ========================================================================

    /**
     * Get host share percentage for a tier and revenue type
     * @param Tier The host tier
     * @param RevenueType Esports or Social
     * @param bIncludeBonus Whether to include badge bonuses
     * @return Share percentage (15-30%)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static float GetHostSharePercent(EDeskillzHostTier Tier, EDeskillzRoomRevenueType RevenueType, bool bIncludeBonus = true);

    /**
     * Get current host's share percentage
     * @param RevenueType Esports or Social
     * @return Share percentage including bonuses
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    float GetCurrentSharePercent(EDeskillzRoomRevenueType RevenueType) const;

    /**
     * Get tier configuration
     * @param Tier The tier to get config for
     * @return Tier configuration
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    static FDeskillzHostTierConfig GetTierConfig(EDeskillzHostTier Tier);

    /**
     * Get current esports tier
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    EDeskillzHostTier GetEsportsTier() const { return CurrentProfile.EsportsTier; }

    /**
     * Get current social tier
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    EDeskillzHostTier GetSocialTier() const { return CurrentProfile.SocialTier; }

    /**
     * Get tier progress percentage (0-100)
     * @param RevenueType Which tier to check progress for
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    float GetTierProgress(EDeskillzRoomRevenueType RevenueType) const;

    // ========================================================================
    // Earnings
    // ========================================================================

    /**
     * Load host earnings summary
     * Results returned via OnEarningsLoaded delegate
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void LoadEarnings();

    /**
     * Get cached earnings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    FDeskillzHostEarnings GetEarnings() const { return CurrentEarnings; }

    /**
     * Get available balance for withdrawal
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    double GetAvailableBalance() const { return CurrentEarnings.Available; }

    /**
     * Get pending balance (unsettled)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    double GetPendingBalance() const { return CurrentEarnings.Pending; }

    /**
     * Request a withdrawal
     * @param Amount Amount to withdraw
     * @param WalletAddress Destination wallet
     * @param Currency Currency (USDT, USDC, etc.)
     * @param Network Blockchain network (BSC, TRON)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void RequestWithdrawal(double Amount, const FString& WalletAddress, const FString& Currency = TEXT("USDT"), const FString& Network = TEXT("BSC"));

    /**
     * Get withdrawal history
     * @param Limit Number of records to fetch
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void GetWithdrawalHistory(int32 Limit = 20);

    // ========================================================================
    // Badges & Bonuses
    // ========================================================================

    /**
     * Get all earned badges
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    TArray<FDeskillzHostBadge> GetBadges() const { return CurrentProfile.Badges; }

    /**
     * Get active badges (not expired)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    TArray<FDeskillzHostBadge> GetActiveBadges() const;

    /**
     * Calculate total bonus percentage from badges
     * @return Total bonus (capped at 10%)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    float CalculateTotalBonus() const;

    /**
     * Check if host has a specific badge
     * @param BadgeCode Badge code to check
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    bool HasBadge(const FString& BadgeCode) const;

    // ========================================================================
    // Active Rooms
    // ========================================================================

    /**
     * Get active rooms hosted by this user
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void GetActiveRooms();

    /**
     * Get cached active rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    TArray<FDeskillzActiveRoomSummary> GetCachedActiveRooms() const { return ActiveRooms; }

    /**
     * Get number of active rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    int32 GetActiveRoomCount() const { return ActiveRooms.Num(); }

    // ========================================================================
    // Stats
    // ========================================================================

    /**
     * Load host statistics
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void LoadStats();

    /**
     * Get cached stats
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    FDeskillzHostStats GetStats() const { return CurrentStats; }

    // ========================================================================
    // Notifications
    // ========================================================================

    /**
     * Load host notifications
     * @param bUnreadOnly Only fetch unread notifications
     * @param Limit Number to fetch
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void LoadNotifications(bool bUnreadOnly = false, int32 Limit = 50);

    /**
     * Mark notification as read
     * @param NotificationId ID of notification to mark
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void MarkNotificationRead(const FString& NotificationId);

    /**
     * Mark all notifications as read
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void MarkAllNotificationsRead();

    /**
     * Get notification settings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host")
    FDeskillzHostNotificationSettings GetNotificationSettings() const { return NotificationSettings; }

    /**
     * Update notification settings
     * @param Settings New settings
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
    void UpdateNotificationSettings(const FDeskillzHostNotificationSettings& Settings);

    // ========================================================================
    // Events / Delegates
    // ========================================================================

    /** Called when host profile is loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostProfileLoaded OnProfileLoaded;

    /** Called when earnings are loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostEarningsLoaded OnEarningsLoaded;

    /** Called when tier changes */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostTierChanged OnTierChanged;

    /** Called when a badge is earned */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostBadgeEarned OnBadgeEarned;

    /** Called when settlement is received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostSettlementReceived OnSettlementReceived;

    /** Called when withdrawal completes */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostWithdrawalComplete OnWithdrawalComplete;

    /** Called when active rooms list updates */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostActiveRoomsUpdated OnActiveRoomsUpdated;

    /** Called when stats are loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostStatsLoaded OnStatsLoaded;

    /** Called when notifications are loaded */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Events")
    FOnDeskillzHostNotificationsLoaded OnNotificationsLoaded;

    // ========================================================================
    // Constants
    // ========================================================================

    /** Maximum bonus percentage from badges */
    static constexpr float MAX_BONUS_PERCENT = 10.0f;

    /** Minimum withdrawal amount */
    static constexpr double MIN_WITHDRAWAL_AMOUNT = 10.0;

    /** Tier evaluation period in days */
    static constexpr int32 TIER_EVALUATION_DAYS = 30;

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Reference to main SDK */
    UPROPERTY()
    UDeskillzSDK* SDK = nullptr;

    /** Current host profile */
    UPROPERTY()
    FDeskillzHostProfile CurrentProfile;

    /** Current earnings */
    UPROPERTY()
    FDeskillzHostEarnings CurrentEarnings;

    /** Current stats */
    UPROPERTY()
    FDeskillzHostStats CurrentStats;

    /** Active rooms */
    UPROPERTY()
    TArray<FDeskillzActiveRoomSummary> ActiveRooms;

    /** Notification settings */
    UPROPERTY()
    FDeskillzHostNotificationSettings NotificationSettings;

    /** Whether profile is loaded */
    bool bProfileLoaded = false;

    /** Tier configs cache */
    static TMap<EDeskillzHostTier, FDeskillzHostTierConfig> TierConfigs;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Initialize tier configurations */
    static void InitializeTierConfigs();

    /** Handle WebSocket messages for real-time updates */
    void HandleWebSocketMessage(const FString& Message);

    /** Parse profile from JSON */
    void ParseProfileFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse earnings from JSON */
    void ParseEarningsFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse badge from JSON */
    FDeskillzHostBadge ParseBadgeFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Calculate share for tier */
    static float CalculateBaseShare(EDeskillzHostTier Tier, EDeskillzRoomRevenueType RevenueType);
};