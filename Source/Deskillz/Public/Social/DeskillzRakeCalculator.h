// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzSocialTypes.h"
#include "DeskillzHostTypes.h"
#include "DeskillzRakeCalculator.generated.h"

/**
 * Rake calculation result
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzRakeResult
{
    GENERATED_BODY()

    /** Total pot before rake */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double GrossPot = 0.0;

    /** Rake amount */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double RakeAmount = 0.0;

    /** Net pot after rake */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double NetPot = 0.0;

    /** Host share of rake */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double HostShare = 0.0;

    /** Platform share of rake */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double PlatformShare = 0.0;

    /** Effective rake percentage used */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    float EffectiveRakePercent = 0.0f;

    /** Whether cap was applied */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    bool bCapApplied = false;

    FDeskillzRakeResult() = default;
};

/**
 * Session revenue summary
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSessionRevenue
{
    GENERATED_BODY()

    /** Total rake collected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double TotalRake = 0.0;

    /** Total host earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double HostEarnings = 0.0;

    /** Total platform earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double PlatformEarnings = 0.0;

    /** Number of rounds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    int32 RoundCount = 0;

    /** Average rake per round */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double AvgRakePerRound = 0.0;

    FDeskillzSessionRevenue() = default;
};

/**
 * Revenue preview for settings
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzRevenuePreview
{
    GENERATED_BODY()

    /** Point value used */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double PointValue = 1.0;

    /** Rake percent used */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    float RakePercent = 5.0f;

    /** Host share percent */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    float HostSharePercent = 15.0f;

    /** Estimated rake per round (for average pot) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double EstRakePerRound = 0.0;

    /** Estimated host earnings per round */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double EstHostPerRound = 0.0;

    /** Estimated earnings per 10 rounds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double EstEarningsPer10Rounds = 0.0;

    /** Estimated earnings per hour (assuming 6 rounds/hour) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rake")
    double EstEarningsPerHour = 0.0;

    FDeskillzRevenuePreview() = default;
};

/**
 * Deskillz Rake Calculator
 * 
 * Calculates rake amounts, host shares, and revenue projections
 * for social games. Supports configurable rake percentages,
 * caps, and the 6-tier host revenue sharing system.
 * 
 * Rake Formula:
 * - RakeAmount = min(Pot * RakePercent, RakeCap)
 * - HostShare = RakeAmount * HostSharePercent
 * - PlatformShare = RakeAmount - HostShare
 * 
 * Usage:
 * - CalculateRake() for individual pot calculations
 * - GetRevenuePreview() for settings UI
 * - GetSessionRevenue() for session summaries
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzRakeCalculator : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzRakeCalculator();

    // ========================================================================
    // Rake Calculation
    // ========================================================================

    /**
     * Calculate rake for a pot
     * @param PotAmount Pot amount in dollars
     * @param RakePercent Rake percentage (1-10)
     * @param RakeCap Maximum rake amount
     * @param HostTier Host's tier for share calculation
     * @param RevenueType Esports or Social
     * @param BonusPercent Additional badge bonus
     * @return Rake calculation result
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static FDeskillzRakeResult CalculateRake(
        double PotAmount,
        float RakePercent,
        double RakeCap,
        EDeskillzHostTier HostTier,
        EDeskillzRoomRevenueType RevenueType = EDeskillzRoomRevenueType::Social,
        float BonusPercent = 0.0f
    );

    /**
     * Calculate rake with settings struct
     * @param PotAmount Pot amount in dollars
     * @param Settings Game settings
     * @param HostTier Host's tier
     * @param BonusPercent Badge bonus
     * @return Rake result
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static FDeskillzRakeResult CalculateRakeWithSettings(
        double PotAmount,
        const FDeskillzSocialGameSettings& Settings,
        EDeskillzHostTier HostTier,
        float BonusPercent = 0.0f
    );

    /**
     * Calculate rake for a point-based pot
     * @param PotPoints Pot in points
     * @param PointValue Value per point in dollars
     * @param RakePercent Rake percentage
     * @param RakeCap Maximum rake
     * @param HostTier Host tier
     * @param BonusPercent Badge bonus
     * @return Rake result
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static FDeskillzRakeResult CalculateRakeFromPoints(
        int32 PotPoints,
        double PointValue,
        float RakePercent,
        double RakeCap,
        EDeskillzHostTier HostTier,
        float BonusPercent = 0.0f
    );

    // ========================================================================
    // Revenue Preview
    // ========================================================================

    /**
     * Get revenue preview for settings
     * @param Settings Game settings
     * @param HostTier Host tier
     * @param BonusPercent Badge bonus
     * @param AvgPotPoints Average pot size in points
     * @param PlayerCount Number of players
     * @return Revenue preview
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static FDeskillzRevenuePreview GetRevenuePreview(
        const FDeskillzSocialGameSettings& Settings,
        EDeskillzHostTier HostTier,
        float BonusPercent = 0.0f,
        int32 AvgPotPoints = 200,
        int32 PlayerCount = 4
    );

    /**
     * Estimate earnings for a session
     * @param Settings Game settings
     * @param HostTier Host tier
     * @param BonusPercent Badge bonus
     * @param ExpectedRounds Expected number of rounds
     * @param AvgPotPoints Average pot in points
     * @return Estimated host earnings
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static double EstimateSessionEarnings(
        const FDeskillzSocialGameSettings& Settings,
        EDeskillzHostTier HostTier,
        float BonusPercent,
        int32 ExpectedRounds,
        int32 AvgPotPoints
    );

    // ========================================================================
    // Session Revenue
    // ========================================================================

    /**
     * Calculate total session revenue
     * @param RoundResults Array of round results
     * @param HostTier Host tier
     * @param BonusPercent Badge bonus
     * @return Session revenue summary
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static FDeskillzSessionRevenue CalculateSessionRevenue(
        const TArray<FDeskillzSocialRoundResult>& RoundResults,
        EDeskillzHostTier HostTier,
        float BonusPercent = 0.0f
    );

    // ========================================================================
    // Share Calculations
    // ========================================================================

    /**
     * Get host share percentage for tier
     * @param Tier Host tier
     * @param RevenueType Esports or Social
     * @return Base share percentage
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Rake")
    static float GetBaseHostShare(EDeskillzHostTier Tier, EDeskillzRoomRevenueType RevenueType);

    /**
     * Calculate total host share with bonus
     * @param Tier Host tier
     * @param RevenueType Revenue type
     * @param BonusPercent Badge bonus
     * @return Total share percentage (capped at 38%)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Rake")
    static float GetTotalHostShare(EDeskillzHostTier Tier, EDeskillzRoomRevenueType RevenueType, float BonusPercent);

    /**
     * Split rake between host and platform
     * @param RakeAmount Total rake
     * @param HostSharePercent Host's percentage
     * @param OutHostShare Host's share (output)
     * @param OutPlatformShare Platform's share (output)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static void SplitRake(
        double RakeAmount,
        float HostSharePercent,
        double& OutHostShare,
        double& OutPlatformShare
    );

    // ========================================================================
    // Validation
    // ========================================================================

    /**
     * Validate rake settings
     * @param RakePercent Rake percentage to validate
     * @param RakeCap Rake cap to validate
     * @param OutError Error message if invalid
     * @return Whether settings are valid
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Rake")
    static bool ValidateRakeSettings(float RakePercent, double RakeCap, FString& OutError);

    /**
     * Get valid rake percent range
     * @param OutMin Minimum percentage
     * @param OutMax Maximum percentage
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Rake")
    static void GetRakePercentRange(float& OutMin, float& OutMax);

    /**
     * Get valid rake cap range
     * @param OutMin Minimum cap
     * @param OutMax Maximum cap
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Rake")
    static void GetRakeCapRange(double& OutMin, double& OutMax);

    // ========================================================================
    // Constants
    // ========================================================================

    /** Minimum rake percentage */
    static constexpr float MIN_RAKE_PERCENT = 1.0f;

    /** Maximum rake percentage */
    static constexpr float MAX_RAKE_PERCENT = 10.0f;

    /** Default rake percentage */
    static constexpr float DEFAULT_RAKE_PERCENT = 5.0f;

    /** Minimum rake cap */
    static constexpr double MIN_RAKE_CAP = 1.0;

    /** Maximum rake cap */
    static constexpr double MAX_RAKE_CAP = 100.0;

    /** Default rake cap */
    static constexpr double DEFAULT_RAKE_CAP = 50.0;

    /** Maximum host share (base + bonus) */
    static constexpr float MAX_HOST_SHARE = 38.0f;

    /** Maximum badge bonus */
    static constexpr float MAX_BADGE_BONUS = 10.0f;

    /** Assumed rounds per hour for projections */
    static constexpr int32 ROUNDS_PER_HOUR = 6;

private:
    /** Tier share percentages for esports */
    static const TMap<EDeskillzHostTier, float> EsportsShares;

    /** Tier share percentages for social */
    static const TMap<EDeskillzHostTier, float> SocialShares;
};