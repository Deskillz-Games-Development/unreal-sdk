// =============================================================================
// Deskillz SDK for Unreal Engine - Rake Calculator Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "Social/DeskillzRakeCalculator.h"
#include "Host/DeskillzHostManager.h"
#include "Misc/DateTime.h"

UDeskillzRakeCalculator* UDeskillzRakeCalculator::Instance = nullptr;

UDeskillzRakeCalculator::UDeskillzRakeCalculator()
    : bIsInitialized(false)
    , RakePercent(5.0f)
    , RakeCap(3.0f)
    , MinPotForRake(0.0f)
    , TotalRakeCollected(0.0f)
    , SessionRakeCollected(0.0f)
{
}

UDeskillzRakeCalculator* UDeskillzRakeCalculator::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzRakeCalculator>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UDeskillzRakeCalculator::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeskillzRakeCalculator already initialized"));
        return;
    }

    // Load default rake structure
    LoadDefaultRakeStructure();

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DeskillzRakeCalculator initialized"));
}

void UDeskillzRakeCalculator::Shutdown()
{
    if (!bIsInitialized) return;

    bIsInitialized = false;
    RakeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("DeskillzRakeCalculator shutdown"));
}

// =============================================================================
// Configuration
// =============================================================================

void UDeskillzRakeCalculator::SetRakeConfig(float InRakePercent, float InRakeCap)
{
    RakePercent = FMath::Clamp(InRakePercent, 0.0f, 100.0f);
    RakeCap = FMath::Max(0.0f, InRakeCap);

    UE_LOG(LogTemp, Log, TEXT("Rake config set: %.2f%% with cap $%.2f"), RakePercent, RakeCap);
}

void UDeskillzRakeCalculator::SetMinPotForRake(float MinPot)
{
    MinPotForRake = FMath::Max(0.0f, MinPot);
}

void UDeskillzRakeCalculator::SetRakeStructure(const TArray<FDeskillzRakeTier>& Structure)
{
    RakeStructure = Structure;

    // Sort by threshold ascending
    RakeStructure.Sort([](const FDeskillzRakeTier& A, const FDeskillzRakeTier& B)
    {
        return A.PotThreshold < B.PotThreshold;
    });

    UE_LOG(LogTemp, Log, TEXT("Rake structure updated with %d tiers"), RakeStructure.Num());
}

float UDeskillzRakeCalculator::GetRakePercent() const
{
    return RakePercent;
}

float UDeskillzRakeCalculator::GetRakeCap() const
{
    return RakeCap;
}

// =============================================================================
// Rake Calculation
// =============================================================================

float UDeskillzRakeCalculator::CalculateRake(float PotAmount) const
{
    if (PotAmount < MinPotForRake)
    {
        return 0.0f;
    }

    float RakeAmount = 0.0f;

    // Use tiered structure if available
    if (RakeStructure.Num() > 0)
    {
        RakeAmount = CalculateTieredRake(PotAmount);
    }
    else
    {
        // Simple percentage calculation
        RakeAmount = PotAmount * (RakePercent / 100.0f);
    }

    // Apply cap
    if (RakeCap > 0.0f)
    {
        RakeAmount = FMath::Min(RakeAmount, RakeCap);
    }

    return RakeAmount;
}

float UDeskillzRakeCalculator::CalculateTieredRake(float PotAmount) const
{
    float TotalRake = 0.0f;
    float RemainingPot = PotAmount;
    float PreviousThreshold = 0.0f;

    for (const FDeskillzRakeTier& Tier : RakeStructure)
    {
        if (RemainingPot <= 0.0f)
        {
            break;
        }

        float TierAmount = 0.0f;
        
        if (PotAmount > Tier.PotThreshold)
        {
            // Pot exceeds this tier, calculate for the full tier range
            TierAmount = Tier.PotThreshold - PreviousThreshold;
        }
        else
        {
            // Pot is within this tier
            TierAmount = PotAmount - PreviousThreshold;
        }

        if (TierAmount > 0.0f)
        {
            float TierRake = TierAmount * (Tier.RakePercent / 100.0f);
            
            // Apply tier-specific cap if set
            if (Tier.TierCap > 0.0f)
            {
                TierRake = FMath::Min(TierRake, Tier.TierCap);
            }
            
            TotalRake += TierRake;
        }

        PreviousThreshold = Tier.PotThreshold;
        RemainingPot = PotAmount - Tier.PotThreshold;
    }

    return TotalRake;
}

float UDeskillzRakeCalculator::CalculateRakePreview(float PotAmount, float CustomRakePercent, float CustomRakeCap) const
{
    if (PotAmount < MinPotForRake)
    {
        return 0.0f;
    }

    float RakeAmount = PotAmount * (CustomRakePercent / 100.0f);
    
    if (CustomRakeCap > 0.0f)
    {
        RakeAmount = FMath::Min(RakeAmount, CustomRakeCap);
    }

    return RakeAmount;
}

FDeskillzRakeBreakdown UDeskillzRakeCalculator::GetRakeBreakdown(float PotAmount) const
{
    FDeskillzRakeBreakdown Breakdown;
    Breakdown.PotAmount = PotAmount;
    Breakdown.TotalRake = CalculateRake(PotAmount);
    Breakdown.WinnerPayout = PotAmount - Breakdown.TotalRake;
    Breakdown.RakePercent = RakePercent;
    Breakdown.RakeCap = RakeCap;
    Breakdown.bCapApplied = (RakeCap > 0.0f && (PotAmount * RakePercent / 100.0f) > RakeCap);

    // Calculate host and platform shares
    UDeskillzHostManager* HostManager = UDeskillzHostManager::Get();
    if (HostManager)
    {
        int32 HostTier = HostManager->GetCurrentTier();
        FDeskillzHostTierInfo TierInfo = HostManager->GetTierInfo(HostTier);
        
        Breakdown.HostSharePercent = TierInfo.RevenueSharePercent;
        Breakdown.PlatformSharePercent = 100.0f - TierInfo.RevenueSharePercent;
        Breakdown.HostShare = Breakdown.TotalRake * (Breakdown.HostSharePercent / 100.0f);
        Breakdown.PlatformShare = Breakdown.TotalRake * (Breakdown.PlatformSharePercent / 100.0f);
    }
    else
    {
        // Default 50/50 split
        Breakdown.HostSharePercent = 50.0f;
        Breakdown.PlatformSharePercent = 50.0f;
        Breakdown.HostShare = Breakdown.TotalRake * 0.5f;
        Breakdown.PlatformShare = Breakdown.TotalRake * 0.5f;
    }

    return Breakdown;
}

// =============================================================================
// Rake Recording & Statistics
// =============================================================================

void UDeskillzRakeCalculator::RecordRake(const FString& SessionId, float RakeAmount)
{
    if (RakeAmount <= 0.0f)
    {
        return;
    }

    FDeskillzRakeRecord Record;
    Record.SessionId = SessionId;
    Record.RakeAmount = RakeAmount;
    Record.Timestamp = FDateTime::UtcNow();
    
    // Calculate breakdown
    FDeskillzRakeBreakdown Breakdown = GetRakeBreakdown(RakeAmount / (RakePercent / 100.0f)); // Reverse calculate pot
    Record.HostShare = Breakdown.HostShare;
    Record.PlatformShare = Breakdown.PlatformShare;

    RakeHistory.Add(Record);

    // Update totals
    TotalRakeCollected += RakeAmount;
    SessionRakeCollected += RakeAmount;

    // Trim history if too large
    while (RakeHistory.Num() > MaxRakeHistorySize)
    {
        RakeHistory.RemoveAt(0);
    }

    OnRakeCollected.Broadcast(SessionId, RakeAmount, Record.HostShare);
}

void UDeskillzRakeCalculator::ResetSessionRake()
{
    SessionRakeCollected = 0.0f;
}

float UDeskillzRakeCalculator::GetTotalRakeCollected() const
{
    return TotalRakeCollected;
}

float UDeskillzRakeCalculator::GetSessionRakeCollected() const
{
    return SessionRakeCollected;
}

TArray<FDeskillzRakeRecord> UDeskillzRakeCalculator::GetRakeHistory(int32 Limit) const
{
    if (Limit <= 0 || Limit >= RakeHistory.Num())
    {
        return RakeHistory;
    }

    TArray<FDeskillzRakeRecord> Result;
    int32 StartIndex = RakeHistory.Num() - Limit;
    
    for (int32 i = StartIndex; i < RakeHistory.Num(); ++i)
    {
        Result.Add(RakeHistory[i]);
    }

    return Result;
}

FDeskillzRakeStatistics UDeskillzRakeCalculator::GetRakeStatistics(EDeskillzStatsPeriod Period) const
{
    FDeskillzRakeStatistics Stats;
    Stats.Period = Period;

    FDateTime Now = FDateTime::UtcNow();
    FDateTime PeriodStart;

    switch (Period)
    {
        case EDeskillzStatsPeriod::Today:
            PeriodStart = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay());
            break;
        case EDeskillzStatsPeriod::Week:
            PeriodStart = Now - FTimespan::FromDays(7);
            break;
        case EDeskillzStatsPeriod::Month:
            PeriodStart = Now - FTimespan::FromDays(30);
            break;
        case EDeskillzStatsPeriod::Year:
            PeriodStart = Now - FTimespan::FromDays(365);
            break;
        case EDeskillzStatsPeriod::AllTime:
            PeriodStart = FDateTime::MinValue();
            break;
    }

    float TotalRake = 0.0f;
    float TotalHostShare = 0.0f;
    float TotalPlatformShare = 0.0f;
    int32 HandCount = 0;

    for (const FDeskillzRakeRecord& Record : RakeHistory)
    {
        if (Record.Timestamp >= PeriodStart)
        {
            TotalRake += Record.RakeAmount;
            TotalHostShare += Record.HostShare;
            TotalPlatformShare += Record.PlatformShare;
            HandCount++;
        }
    }

    Stats.TotalRake = TotalRake;
    Stats.HostShare = TotalHostShare;
    Stats.PlatformShare = TotalPlatformShare;
    Stats.HandCount = HandCount;
    Stats.AverageRakePerHand = HandCount > 0 ? TotalRake / HandCount : 0.0f;

    return Stats;
}

// =============================================================================
// Revenue Estimation
// =============================================================================

float UDeskillzRakeCalculator::EstimateHostEarnings(float ExpectedPotSize, int32 ExpectedHands, int32 HostTier) const
{
    float ExpectedRake = CalculateRake(ExpectedPotSize) * ExpectedHands;
    
    UDeskillzHostManager* HostManager = UDeskillzHostManager::Get();
    float HostSharePercent = 50.0f;
    
    if (HostManager)
    {
        FDeskillzHostTierInfo TierInfo = HostManager->GetTierInfo(HostTier);
        HostSharePercent = TierInfo.RevenueSharePercent;
    }

    return ExpectedRake * (HostSharePercent / 100.0f);
}

FDeskillzRevenueProjection UDeskillzRakeCalculator::ProjectRevenue(
    float PointValue, 
    int32 ExpectedPlayers, 
    int32 ExpectedHoursPerDay,
    int32 DaysToProject) const
{
    FDeskillzRevenueProjection Projection;
    Projection.PointValue = PointValue;
    Projection.ExpectedPlayers = ExpectedPlayers;
    Projection.ProjectionDays = DaysToProject;

    // Assumptions:
    // - Average pot is 10x point value
    // - About 30 hands per hour with full table
    // - Rake collected on 70% of hands (some small pots below threshold)

    float AveragePot = PointValue * 10.0f;
    int32 HandsPerHour = 30;
    float RakeableHandsPercent = 0.7f;

    int32 TotalHands = HandsPerHour * ExpectedHoursPerDay * DaysToProject;
    int32 RakeableHands = FMath::RoundToInt(TotalHands * RakeableHandsPercent);

    float TotalRake = 0.0f;
    for (int32 i = 0; i < RakeableHands; ++i)
    {
        TotalRake += CalculateRake(AveragePot);
    }

    Projection.ProjectedTotalRake = TotalRake;
    Projection.ProjectedDailyRake = TotalRake / DaysToProject;

    // Calculate host earnings based on current tier
    UDeskillzHostManager* HostManager = UDeskillzHostManager::Get();
    float HostSharePercent = 50.0f;
    
    if (HostManager)
    {
        int32 CurrentTier = HostManager->GetCurrentTier();
        FDeskillzHostTierInfo TierInfo = HostManager->GetTierInfo(CurrentTier);
        HostSharePercent = TierInfo.RevenueSharePercent;
    }

    Projection.ProjectedHostEarnings = TotalRake * (HostSharePercent / 100.0f);
    Projection.ProjectedDailyHostEarnings = Projection.ProjectedHostEarnings / DaysToProject;

    return Projection;
}

// =============================================================================
// Private Methods
// =============================================================================

void UDeskillzRakeCalculator::LoadDefaultRakeStructure()
{
    // Default tiered rake structure (poker-style)
    RakeStructure.Empty();

    FDeskillzRakeTier Tier1;
    Tier1.PotThreshold = 10.0f;
    Tier1.RakePercent = 5.0f;
    Tier1.TierCap = 1.0f;
    RakeStructure.Add(Tier1);

    FDeskillzRakeTier Tier2;
    Tier2.PotThreshold = 50.0f;
    Tier2.RakePercent = 4.0f;
    Tier2.TierCap = 2.0f;
    RakeStructure.Add(Tier2);

    FDeskillzRakeTier Tier3;
    Tier3.PotThreshold = 100.0f;
    Tier3.RakePercent = 3.0f;
    Tier3.TierCap = 3.0f;
    RakeStructure.Add(Tier3);

    FDeskillzRakeTier Tier4;
    Tier4.PotThreshold = 500.0f;
    Tier4.RakePercent = 2.5f;
    Tier4.TierCap = 5.0f;
    RakeStructure.Add(Tier4);
}