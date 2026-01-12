// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "DeskillzHostTypes.generated.h"

// ============================================================================
// Host Tier System
// ============================================================================

/**
 * Host tier levels with associated revenue sharing percentages
 */
UENUM(BlueprintType)
enum class EDeskillzHostTier : uint8
{
    Bronze      UMETA(DisplayName = "Bronze (15%)"),
    Silver      UMETA(DisplayName = "Silver (18%)"),
    Gold        UMETA(DisplayName = "Gold (20%)"),
    Platinum    UMETA(DisplayName = "Platinum (23%)"),
    Diamond     UMETA(DisplayName = "Diamond (25%)"),
    Elite       UMETA(DisplayName = "Elite (28%)")
};

/**
 * Room revenue type for tier calculations
 */
UENUM(BlueprintType)
enum class EDeskillzRoomRevenueType : uint8
{
    Esports     UMETA(DisplayName = "Esports"),
    Social      UMETA(DisplayName = "Social Games")
};

/**
 * Badge categories
 */
UENUM(BlueprintType)
enum class EDeskillzBadgeCategory : uint8
{
    Achievement     UMETA(DisplayName = "Achievement"),
    Performance     UMETA(DisplayName = "Performance"),
    Exclusive       UMETA(DisplayName = "Exclusive")
};

/**
 * Host notification types
 */
UENUM(BlueprintType)
enum class EDeskillzHostNotificationType : uint8
{
    Settlement          UMETA(DisplayName = "Settlement"),
    TierUp              UMETA(DisplayName = "Tier Upgrade"),
    TierDown            UMETA(DisplayName = "Tier Downgrade"),
    BadgeEarned         UMETA(DisplayName = "Badge Earned"),
    WithdrawalComplete  UMETA(DisplayName = "Withdrawal Complete"),
    RoomActivity        UMETA(DisplayName = "Room Activity"),
    System              UMETA(DisplayName = "System")
};

/**
 * Withdrawal status
 */
UENUM(BlueprintType)
enum class EDeskillzWithdrawalStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Processing  UMETA(DisplayName = "Processing"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Cancelled   UMETA(DisplayName = "Cancelled")
};

// ============================================================================
// Host Tier Configuration
// ============================================================================

/**
 * Configuration for a host tier level
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostTierConfig
{
    GENERATED_BODY()

    /** Tier level */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzHostTier Tier = EDeskillzHostTier::Bronze;

    /** Display name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString DisplayName;

    /** Base revenue share percentage */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float BaseSharePercent = 15.0f;

    /** Esports share percentage */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float EsportsSharePercent = 15.0f;

    /** Social games share percentage */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float SocialSharePercent = 18.0f;

    /** Minimum unique players to qualify */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 RequiredUniquePlayers = 0;

    /** Icon identifier */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Icon;

    /** Theme color (hex) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FLinearColor ThemeColor = FLinearColor::White;

    FDeskillzHostTierConfig() = default;

    static FDeskillzHostTierConfig GetBronze()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Bronze;
        Config.DisplayName = TEXT("Bronze");
        Config.BaseSharePercent = 15.0f;
        Config.EsportsSharePercent = 15.0f;
        Config.SocialSharePercent = 18.0f;
        Config.RequiredUniquePlayers = 0;
        Config.Icon = TEXT("[B]");
        Config.ThemeColor = FLinearColor(0.8f, 0.5f, 0.2f);
        return Config;
    }

    static FDeskillzHostTierConfig GetSilver()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Silver;
        Config.DisplayName = TEXT("Silver");
        Config.BaseSharePercent = 18.0f;
        Config.EsportsSharePercent = 18.0f;
        Config.SocialSharePercent = 20.0f;
        Config.RequiredUniquePlayers = 50;
        Config.Icon = TEXT("[S]");
        Config.ThemeColor = FLinearColor(0.75f, 0.75f, 0.75f);
        return Config;
    }

    static FDeskillzHostTierConfig GetGold()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Gold;
        Config.DisplayName = TEXT("Gold");
        Config.BaseSharePercent = 20.0f;
        Config.EsportsSharePercent = 20.0f;
        Config.SocialSharePercent = 22.0f;
        Config.RequiredUniquePlayers = 200;
        Config.Icon = TEXT("[G]");
        Config.ThemeColor = FLinearColor(1.0f, 0.84f, 0.0f);
        return Config;
    }

    static FDeskillzHostTierConfig GetPlatinum()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Platinum;
        Config.DisplayName = TEXT("Platinum");
        Config.BaseSharePercent = 23.0f;
        Config.EsportsSharePercent = 23.0f;
        Config.SocialSharePercent = 25.0f;
        Config.RequiredUniquePlayers = 500;
        Config.Icon = TEXT("[P]");
        Config.ThemeColor = FLinearColor(0.9f, 0.9f, 1.0f);
        return Config;
    }

    static FDeskillzHostTierConfig GetDiamond()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Diamond;
        Config.DisplayName = TEXT("Diamond");
        Config.BaseSharePercent = 25.0f;
        Config.EsportsSharePercent = 25.0f;
        Config.SocialSharePercent = 27.0f;
        Config.RequiredUniquePlayers = 1000;
        Config.Icon = TEXT("[D]");
        Config.ThemeColor = FLinearColor(0.6f, 0.85f, 1.0f);
        return Config;
    }

    static FDeskillzHostTierConfig GetElite()
    {
        FDeskillzHostTierConfig Config;
        Config.Tier = EDeskillzHostTier::Elite;
        Config.DisplayName = TEXT("Elite");
        Config.BaseSharePercent = 28.0f;
        Config.EsportsSharePercent = 28.0f;
        Config.SocialSharePercent = 30.0f;
        Config.RequiredUniquePlayers = 2500;
        Config.Icon = TEXT("[E]");
        Config.ThemeColor = FLinearColor(0.8f, 0.4f, 1.0f);
        return Config;
    }
};

// ============================================================================
// Host Profile & Badge
// ============================================================================

/**
 * Host badge information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostBadge
{
    GENERATED_BODY()

    /** Unique badge ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString BadgeId;

    /** Badge code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Code;

    /** Display name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Name;

    /** Description */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Description;

    /** Badge category */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzBadgeCategory Category = EDeskillzBadgeCategory::Achievement;

    /** Icon URL */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString IconUrl;

    /** Bonus percentage (0-2%) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float BonusPercent = 0.0f;

    /** When earned */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime EarnedAt;

    /** Expiration date (for performance badges) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime ExpiresAt;

    /** Whether badge is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    bool bIsActive = true;

    FDeskillzHostBadge() = default;

    bool IsExpired() const
    {
        return ExpiresAt > FDateTime::MinValue() && FDateTime::UtcNow() > ExpiresAt;
    }
};

/**
 * Host profile information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostProfile
{
    GENERATED_BODY()

    /** Host user ID (odid) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString HostId;

    /** Username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Username;

    /** Avatar URL */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString AvatarUrl;

    /** Current esports tier */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzHostTier EsportsTier = EDeskillzHostTier::Bronze;

    /** Current social tier */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzHostTier SocialTier = EDeskillzHostTier::Bronze;

    /** Host level */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 Level = 1;

    /** Experience points */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 Experience = 0;

    /** Whether age verified (18+) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    bool bIsVerified = false;

    /** Current hosting streak (days) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 CurrentStreak = 0;

    /** Longest streak ever */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 LongestStreak = 0;

    /** Average rating (1-5 stars) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float Rating = 0.0f;

    /** Number of ratings received */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 RatingCount = 0;

    /** Total rooms created */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 TotalRoomsCreated = 0;

    /** Total players hosted */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 TotalPlayersHosted = 0;

    /** Total earnings (all-time) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double TotalEarnings = 0.0;

    /** Earned badges */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    TArray<FDeskillzHostBadge> Badges;

    /** When profile was created */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime CreatedAt;

    FDeskillzHostProfile() = default;

    /** Get tier name */
    static FString GetTierName(EDeskillzHostTier Tier)
    {
        switch (Tier)
        {
            case EDeskillzHostTier::Bronze:   return TEXT("Bronze");
            case EDeskillzHostTier::Silver:   return TEXT("Silver");
            case EDeskillzHostTier::Gold:     return TEXT("Gold");
            case EDeskillzHostTier::Platinum: return TEXT("Platinum");
            case EDeskillzHostTier::Diamond:  return TEXT("Diamond");
            case EDeskillzHostTier::Elite:    return TEXT("Elite");
            default: return TEXT("Unknown");
        }
    }

    /** Get level title */
    FString GetLevelTitle() const
    {
        if (Level < 5) return TEXT("Newcomer");
        if (Level < 10) return TEXT("Regular");
        if (Level < 20) return TEXT("Experienced");
        if (Level < 35) return TEXT("Veteran");
        if (Level < 50) return TEXT("Expert");
        return TEXT("Master");
    }

    /** Get highest tier between esports and social */
    EDeskillzHostTier GetHighestTier() const
    {
        return static_cast<int32>(EsportsTier) > static_cast<int32>(SocialTier) ? EsportsTier : SocialTier;
    }
};

// ============================================================================
// Host Earnings
// ============================================================================

/**
 * Host transaction record
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostTransaction
{
    GENERATED_BODY()

    /** Transaction ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString TransactionId;

    /** Transaction type */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Type;

    /** Amount (positive = credit, negative = debit) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Amount = 0.0;

    /** Description */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Description;

    /** Related room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString RoomId;

    /** Transaction timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime CreatedAt;

    FDeskillzHostTransaction() = default;
};

/**
 * Host earnings summary
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostEarnings
{
    GENERATED_BODY()

    /** Available balance (withdrawable) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Available = 0.0;

    /** Pending balance (not yet settled) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Pending = 0.0;

    /** Earnings today */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Today = 0.0;

    /** Earnings this week */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double ThisWeek = 0.0;

    /** Earnings this month */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double ThisMonth = 0.0;

    /** All-time earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double AllTime = 0.0;

    /** Esports earnings (all-time) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double EsportsEarnings = 0.0;

    /** Social game earnings (all-time) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double SocialEarnings = 0.0;

    /** Bonus earnings (all-time) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double BonusEarnings = 0.0;

    /** Recent transactions */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    TArray<FDeskillzHostTransaction> RecentTransactions;

    FDeskillzHostEarnings() = default;

    /** Get total balance */
    double GetTotal() const { return Available + Pending; }
};

// ============================================================================
// Active Room & Stats
// ============================================================================

/**
 * Summary of an active room hosted
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzActiveRoomSummary
{
    GENERATED_BODY()

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString RoomId;

    /** Room code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString RoomCode;

    /** Room name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Name;

    /** Game ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString GameId;

    /** Current player count */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 CurrentPlayers = 0;

    /** Max players */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 MaxPlayers = 0;

    /** Current round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 CurrentRound = 0;

    /** Whether game is in progress */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    bool bIsPlaying = false;

    /** Estimated earnings from this room */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double EstimatedEarnings = 0.0;

    /** When room was created */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime CreatedAt;

    FDeskillzActiveRoomSummary() = default;
};

/**
 * Host statistics
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostStats
{
    GENERATED_BODY()

    /** Rooms created this month */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 RoomsThisMonth = 0;

    /** Players hosted this month */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 PlayersThisMonth = 0;

    /** Unique players this month */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 UniquePlayersThisMonth = 0;

    /** Average players per room */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float AvgPlayersPerRoom = 0.0f;

    /** Room completion rate */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float CompletionRate = 0.0f;

    /** Tier progress percentage (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    float TierProgress = 0.0f;

    /** Days until tier evaluation */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    int32 DaysUntilTierReset = 0;

    FDeskillzHostStats() = default;
};

// ============================================================================
// Host Notifications
// ============================================================================

/**
 * Host notification
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostNotification
{
    GENERATED_BODY()

    /** Notification ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString NotificationId;

    /** Notification type */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzHostNotificationType Type = EDeskillzHostNotificationType::System;

    /** Title */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Title;

    /** Message body */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Message;

    /** Whether read */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    bool bIsRead = false;

    /** Related room ID (if applicable) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString RoomId;

    /** Related amount (if applicable) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Amount = 0.0;

    /** When created */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime CreatedAt;

    FDeskillzHostNotification() = default;
};

/**
 * Host notification settings
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostNotificationSettings
{
    GENERATED_BODY()

    /** Email notifications enabled */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bEmailEnabled = true;

    /** Push notifications enabled */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bPushEnabled = true;

    /** Settlement notifications */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bSettlementNotifications = true;

    /** Tier change notifications */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bTierNotifications = true;

    /** Badge earned notifications */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bBadgeNotifications = true;

    /** Room activity notifications */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    bool bRoomActivityNotifications = false;

    /** Minimum settlement amount for notification */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Host")
    double MinSettlementNotification = 1.0;

    FDeskillzHostNotificationSettings() = default;
};

// ============================================================================
// Withdrawal
// ============================================================================

/**
 * Withdrawal request
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWithdrawalRequest
{
    GENERATED_BODY()

    /** Withdrawal ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString WithdrawalId;

    /** Amount to withdraw */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    double Amount = 0.0;

    /** Destination wallet address */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString WalletAddress;

    /** Currency */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Currency = TEXT("USDT");

    /** Network (e.g., BSC, TRON) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString Network = TEXT("BSC");

    /** Status */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    EDeskillzWithdrawalStatus Status = EDeskillzWithdrawalStatus::Pending;

    /** Transaction hash (when completed) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FString TransactionHash;

    /** When requested */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime RequestedAt;

    /** When completed */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
    FDateTime CompletedAt;

    FDeskillzWithdrawalRequest() = default;
};

// ============================================================================
// Callback Delegates
// ============================================================================

/** Delegate for host profile loaded */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostProfileLoaded, const FDeskillzHostProfile&, Profile, const FDeskillzError&, Error);

/** Delegate for host earnings loaded */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostEarningsLoaded, const FDeskillzHostEarnings&, Earnings, const FDeskillzError&, Error);

/** Delegate for host tier changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostTierChanged, EDeskillzHostTier, OldTier, EDeskillzHostTier, NewTier, EDeskillzRoomRevenueType, RevenueType);

/** Delegate for badge earned */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostBadgeEarned, const FDeskillzHostBadge&, Badge);

/** Delegate for settlement received */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzHostSettlementReceived, const FString&, RoomId, double, Amount, const FString&, Trigger);

/** Delegate for withdrawal completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzHostWithdrawalComplete, const FDeskillzWithdrawalRequest&, Withdrawal, const FDeskillzError&, Error);

/** Delegate for active rooms updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostActiveRoomsUpdated, const TArray<FDeskillzActiveRoomSummary>&, Rooms);

/** Delegate for stats loaded */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostStatsLoaded, const FDeskillzHostStats&, Stats);

/** Delegate for notifications loaded */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzHostNotificationsLoaded, const TArray<FDeskillzHostNotification>&, Notifications);