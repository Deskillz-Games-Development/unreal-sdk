// =============================================================================
// Deskillz SDK for Unreal Engine - Buy-In Manager Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "Social/DeskillzBuyInManager.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UDeskillzBuyInManager* UDeskillzBuyInManager::Instance = nullptr;

UDeskillzBuyInManager::UDeskillzBuyInManager()
    : bIsInitialized(false)
{
    // Default configuration
    CurrentConfig.MinBuyIn = 10.0f;
    CurrentConfig.MaxBuyIn = 200.0f;
    CurrentConfig.PointValue = 0.01f;
    CurrentConfig.bAllowRebuy = true;
    CurrentConfig.RebuyPeriodRounds = 0; // 0 = unlimited
    CurrentConfig.MaxRebuysPerPlayer = 5;
    CurrentConfig.RebuyMinBalance = 0.0f;
}

UDeskillzBuyInManager* UDeskillzBuyInManager::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzBuyInManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UDeskillzBuyInManager::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeskillzBuyInManager already initialized"));
        return;
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DeskillzBuyInManager initialized"));
}

void UDeskillzBuyInManager::Shutdown()
{
    if (!bIsInitialized) return;

    PlayerBuyInRecords.Empty();
    PendingTransactions.Empty();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("DeskillzBuyInManager shutdown"));
}

// =============================================================================
// Configuration
// =============================================================================

void UDeskillzBuyInManager::SetBuyInConfig(const FDeskillzBuyInConfig& Config)
{
    CurrentConfig = Config;
    
    UE_LOG(LogTemp, Log, TEXT("Buy-in config set: Min=$%.2f, Max=$%.2f, PointValue=$%.4f"), 
        Config.MinBuyIn, Config.MaxBuyIn, Config.PointValue);
}

FDeskillzBuyInConfig UDeskillzBuyInManager::GetBuyInConfig() const
{
    return CurrentConfig;
}

void UDeskillzBuyInManager::SetRebuyEnabled(bool bEnabled)
{
    CurrentConfig.bAllowRebuy = bEnabled;
}

void UDeskillzBuyInManager::SetRebuyPeriod(int32 MaxRounds)
{
    CurrentConfig.RebuyPeriodRounds = MaxRounds;
}

// =============================================================================
// Validation
// =============================================================================

bool UDeskillzBuyInManager::ValidateBuyIn(const FString& PlayerId, float Amount) const
{
    // Check amount is within range
    if (Amount < CurrentConfig.MinBuyIn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Buy-in $%.2f below minimum $%.2f for player %s"), 
            Amount, CurrentConfig.MinBuyIn, *PlayerId);
        return false;
    }

    if (Amount > CurrentConfig.MaxBuyIn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Buy-in $%.2f exceeds maximum $%.2f for player %s"), 
            Amount, CurrentConfig.MaxBuyIn, *PlayerId);
        return false;
    }

    // Check player doesn't already have an active buy-in
    if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        if (Record->bIsActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player %s already has active buy-in"), *PlayerId);
            return false;
        }
    }

    return true;
}

bool UDeskillzBuyInManager::ValidateRebuy(const FString& PlayerId, float Amount, int32 CurrentRound) const
{
    // Check if rebuys are enabled
    if (!CurrentConfig.bAllowRebuy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rebuys not allowed"));
        return false;
    }

    // Get player record
    const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId);
    if (!Record)
    {
        UE_LOG(LogTemp, Warning, TEXT("No buy-in record found for player %s"), *PlayerId);
        return false;
    }

    // Check rebuy count limit
    if (CurrentConfig.MaxRebuysPerPlayer > 0 && Record->RebuyCount >= CurrentConfig.MaxRebuysPerPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s has reached max rebuys (%d)"), 
            *PlayerId, CurrentConfig.MaxRebuysPerPlayer);
        return false;
    }

    // Check rebuy period
    if (CurrentConfig.RebuyPeriodRounds > 0)
    {
        int32 RoundsPlayed = CurrentRound - Record->InitialRound;
        if (RoundsPlayed > CurrentConfig.RebuyPeriodRounds)
        {
            UE_LOG(LogTemp, Warning, TEXT("Rebuy period expired for player %s (round %d, period %d)"), 
                *PlayerId, RoundsPlayed, CurrentConfig.RebuyPeriodRounds);
            return false;
        }
    }

    // Check current balance requirement
    if (CurrentConfig.RebuyMinBalance > 0 && Record->CurrentBalance > CurrentConfig.RebuyMinBalance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s balance $%.2f above rebuy threshold $%.2f"), 
            *PlayerId, Record->CurrentBalance, CurrentConfig.RebuyMinBalance);
        return false;
    }

    // Validate amount
    if (Amount < CurrentConfig.MinBuyIn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rebuy amount $%.2f below minimum $%.2f"), 
            Amount, CurrentConfig.MinBuyIn);
        return false;
    }

    // Check if total would exceed max
    float NewTotal = Record->TotalBuyIn + Amount;
    if (CurrentConfig.MaxTotalBuyIn > 0 && NewTotal > CurrentConfig.MaxTotalBuyIn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rebuy would exceed max total buy-in ($%.2f + $%.2f > $%.2f)"), 
            Record->TotalBuyIn, Amount, CurrentConfig.MaxTotalBuyIn);
        return false;
    }

    return true;
}

FDeskillzBuyInValidationResult UDeskillzBuyInManager::GetValidationResult(const FString& PlayerId, float Amount, bool bIsRebuy) const
{
    FDeskillzBuyInValidationResult Result;
    Result.bIsValid = true;
    Result.RequestedAmount = Amount;
    Result.MinAllowed = CurrentConfig.MinBuyIn;
    Result.MaxAllowed = CurrentConfig.MaxBuyIn;

    if (Amount < CurrentConfig.MinBuyIn)
    {
        Result.bIsValid = false;
        Result.ErrorMessage = FString::Printf(TEXT("Amount below minimum ($%.2f)"), CurrentConfig.MinBuyIn);
    }
    else if (Amount > CurrentConfig.MaxBuyIn)
    {
        Result.bIsValid = false;
        Result.ErrorMessage = FString::Printf(TEXT("Amount exceeds maximum ($%.2f)"), CurrentConfig.MaxBuyIn);
    }

    if (bIsRebuy)
    {
        if (!CurrentConfig.bAllowRebuy)
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("Rebuys not allowed in this room");
        }
        else if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
        {
            if (CurrentConfig.MaxRebuysPerPlayer > 0 && Record->RebuyCount >= CurrentConfig.MaxRebuysPerPlayer)
            {
                Result.bIsValid = false;
                Result.ErrorMessage = FString::Printf(TEXT("Maximum rebuys reached (%d)"), CurrentConfig.MaxRebuysPerPlayer);
            }
        }
    }

    // Calculate chips
    if (Result.bIsValid && CurrentConfig.PointValue > 0)
    {
        Result.ChipsReceived = FMath::FloorToInt(Amount / CurrentConfig.PointValue);
    }

    return Result;
}

// =============================================================================
// Buy-In Processing
// =============================================================================

void UDeskillzBuyInManager::ProcessBuyIn(const FString& PlayerId, float Amount)
{
    // Create transaction
    FDeskillzBuyInTransaction Transaction;
    Transaction.TransactionId = GenerateTransactionId();
    Transaction.PlayerId = PlayerId;
    Transaction.Amount = Amount;
    Transaction.Type = EDeskillzBuyInTransactionType::InitialBuyIn;
    Transaction.Status = EDeskillzTransactionStatus::Pending;
    Transaction.Timestamp = FDateTime::UtcNow();

    PendingTransactions.Add(Transaction.TransactionId, Transaction);

    // Process wallet deduction
    ProcessWalletTransaction(Transaction, [this, PlayerId, Amount](bool bSuccess, const FString& TransactionId)
    {
        if (bSuccess)
        {
            // Create player record
            FDeskillzPlayerBuyInRecord Record;
            Record.PlayerId = PlayerId;
            Record.TotalBuyIn = Amount;
            Record.CurrentBalance = Amount;
            Record.InitialBuyIn = Amount;
            Record.RebuyCount = 0;
            Record.InitialRound = 0; // Will be set by game manager
            Record.bIsActive = true;
            Record.JoinTime = FDateTime::UtcNow();

            PlayerBuyInRecords.Add(PlayerId, Record);

            // Update transaction status
            if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
            {
                Trans->Status = EDeskillzTransactionStatus::Completed;
            }

            int32 Chips = CalculateChips(Amount);
            OnBuyInProcessed.Broadcast(PlayerId, Amount, Chips);
            
            UE_LOG(LogTemp, Log, TEXT("Buy-in processed: Player %s, Amount $%.2f, Chips %d"), 
                *PlayerId, Amount, Chips);
        }
        else
        {
            // Update transaction status
            if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
            {
                Trans->Status = EDeskillzTransactionStatus::Failed;
            }

            OnBuyInFailed.Broadcast(PlayerId, TEXT("Wallet transaction failed"));
        }
    });
}

void UDeskillzBuyInManager::ProcessRebuy(const FString& PlayerId, float Amount)
{
    FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId);
    if (!Record)
    {
        OnRebuyFailed.Broadcast(PlayerId, TEXT("Player not found"));
        return;
    }

    // Create transaction
    FDeskillzBuyInTransaction Transaction;
    Transaction.TransactionId = GenerateTransactionId();
    Transaction.PlayerId = PlayerId;
    Transaction.Amount = Amount;
    Transaction.Type = EDeskillzBuyInTransactionType::Rebuy;
    Transaction.Status = EDeskillzTransactionStatus::Pending;
    Transaction.Timestamp = FDateTime::UtcNow();

    PendingTransactions.Add(Transaction.TransactionId, Transaction);

    // Process wallet deduction
    ProcessWalletTransaction(Transaction, [this, PlayerId, Amount](bool bSuccess, const FString& TransactionId)
    {
        if (bSuccess)
        {
            FDeskillzPlayerBuyInRecord* PlayerRecord = PlayerBuyInRecords.Find(PlayerId);
            if (PlayerRecord)
            {
                PlayerRecord->TotalBuyIn += Amount;
                PlayerRecord->CurrentBalance += Amount;
                PlayerRecord->RebuyCount++;
                PlayerRecord->LastRebuyTime = FDateTime::UtcNow();
            }

            // Update transaction status
            if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
            {
                Trans->Status = EDeskillzTransactionStatus::Completed;
            }

            int32 Chips = CalculateChips(Amount);
            OnRebuyProcessed.Broadcast(PlayerId, Amount, Chips);
            
            UE_LOG(LogTemp, Log, TEXT("Rebuy processed: Player %s, Amount $%.2f, Chips %d"), 
                *PlayerId, Amount, Chips);
        }
        else
        {
            if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
            {
                Trans->Status = EDeskillzTransactionStatus::Failed;
            }

            OnRebuyFailed.Broadcast(PlayerId, TEXT("Wallet transaction failed"));
        }
    });
}

void UDeskillzBuyInManager::ProcessCashOut(const FString& PlayerId, float Amount)
{
    FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId);
    if (!Record)
    {
        OnCashOutFailed.Broadcast(PlayerId, TEXT("Player not found"));
        return;
    }

    if (Amount > Record->CurrentBalance)
    {
        OnCashOutFailed.Broadcast(PlayerId, TEXT("Insufficient balance"));
        return;
    }

    // Create transaction
    FDeskillzBuyInTransaction Transaction;
    Transaction.TransactionId = GenerateTransactionId();
    Transaction.PlayerId = PlayerId;
    Transaction.Amount = Amount;
    Transaction.Type = EDeskillzBuyInTransactionType::CashOut;
    Transaction.Status = EDeskillzTransactionStatus::Pending;
    Transaction.Timestamp = FDateTime::UtcNow();

    PendingTransactions.Add(Transaction.TransactionId, Transaction);

    // Process wallet credit
    ProcessWalletTransaction(Transaction, [this, PlayerId, Amount](bool bSuccess, const FString& TransactionId)
    {
        if (bSuccess)
        {
            FDeskillzPlayerBuyInRecord* PlayerRecord = PlayerBuyInRecords.Find(PlayerId);
            if (PlayerRecord)
            {
                float NetResult = Amount - PlayerRecord->TotalBuyIn;
                PlayerRecord->CurrentBalance = 0.0f;
                PlayerRecord->bIsActive = false;
                PlayerRecord->LeaveTime = FDateTime::UtcNow();

                // Update transaction status
                if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
                {
                    Trans->Status = EDeskillzTransactionStatus::Completed;
                }

                OnCashOutProcessed.Broadcast(PlayerId, Amount, NetResult);
                
                UE_LOG(LogTemp, Log, TEXT("Cash out processed: Player %s, Amount $%.2f, Net $%.2f"), 
                    *PlayerId, Amount, NetResult);
            }
        }
        else
        {
            if (FDeskillzBuyInTransaction* Trans = PendingTransactions.Find(TransactionId))
            {
                Trans->Status = EDeskillzTransactionStatus::Failed;
            }

            OnCashOutFailed.Broadcast(PlayerId, TEXT("Wallet transaction failed"));
        }
    });
}

// =============================================================================
// Balance Management
// =============================================================================

void UDeskillzBuyInManager::UpdatePlayerBalance(const FString& PlayerId, float NewBalance)
{
    if (FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        Record->CurrentBalance = NewBalance;
    }
}

void UDeskillzBuyInManager::AddToPlayerBalance(const FString& PlayerId, float Amount)
{
    if (FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        Record->CurrentBalance += Amount;
    }
}

void UDeskillzBuyInManager::SubtractFromPlayerBalance(const FString& PlayerId, float Amount)
{
    if (FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        Record->CurrentBalance = FMath::Max(0.0f, Record->CurrentBalance - Amount);
    }
}

float UDeskillzBuyInManager::GetPlayerBalance(const FString& PlayerId) const
{
    if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        return Record->CurrentBalance;
    }
    return 0.0f;
}

float UDeskillzBuyInManager::GetPlayerTotalBuyIn(const FString& PlayerId) const
{
    if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        return Record->TotalBuyIn;
    }
    return 0.0f;
}

int32 UDeskillzBuyInManager::GetPlayerRebuyCount(const FString& PlayerId) const
{
    if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        return Record->RebuyCount;
    }
    return 0;
}

FDeskillzPlayerBuyInRecord UDeskillzBuyInManager::GetPlayerRecord(const FString& PlayerId) const
{
    if (const FDeskillzPlayerBuyInRecord* Record = PlayerBuyInRecords.Find(PlayerId))
    {
        return *Record;
    }
    return FDeskillzPlayerBuyInRecord();
}

// =============================================================================
// Utility
// =============================================================================

int32 UDeskillzBuyInManager::CalculateChips(float Amount) const
{
    if (CurrentConfig.PointValue <= 0)
    {
        return 0;
    }
    return FMath::FloorToInt(Amount / CurrentConfig.PointValue);
}

float UDeskillzBuyInManager::CalculateValue(int32 Chips) const
{
    return static_cast<float>(Chips) * CurrentConfig.PointValue;
}

TArray<float> UDeskillzBuyInManager::GetPresetBuyInAmounts() const
{
    TArray<float> Presets;
    
    // Min buy-in
    Presets.Add(CurrentConfig.MinBuyIn);
    
    // 100x point value
    float Preset100x = CurrentConfig.PointValue * 100.0f;
    if (Preset100x > CurrentConfig.MinBuyIn && Preset100x < CurrentConfig.MaxBuyIn)
    {
        Presets.Add(Preset100x);
    }
    
    // 200x point value
    float Preset200x = CurrentConfig.PointValue * 200.0f;
    if (Preset200x > CurrentConfig.MinBuyIn && Preset200x < CurrentConfig.MaxBuyIn)
    {
        Presets.Add(Preset200x);
    }
    
    // Max buy-in
    Presets.Add(CurrentConfig.MaxBuyIn);

    return Presets;
}

// =============================================================================
// Private Methods
// =============================================================================

void UDeskillzBuyInManager::ProcessWalletTransaction(const FDeskillzBuyInTransaction& Transaction, 
    TFunction<void(bool, const FString&)> Callback)
{
    // This would integrate with the wallet system
    // For now, simulate successful transaction
    
    FString TransactionId = Transaction.TransactionId;
    
    // Simulate async wallet call
    // In production, this would make actual wallet API calls
    
    if (Callback)
    {
        Callback(true, TransactionId);
    }
}

FString UDeskillzBuyInManager::GenerateTransactionId() const
{
    return FString::Printf(TEXT("BUYIN_%s"), *FGuid::NewGuid().ToString());
}

void UDeskillzBuyInManager::CleanupExpiredTransactions()
{
    FDateTime CutoffTime = FDateTime::UtcNow() - FTimespan::FromHours(24);
    
    TArray<FString> ExpiredIds;
    for (const auto& Pair : PendingTransactions)
    {
        if (Pair.Value.Timestamp < CutoffTime && 
            Pair.Value.Status != EDeskillzTransactionStatus::Pending)
        {
            ExpiredIds.Add(Pair.Key);
        }
    }

    for (const FString& Id : ExpiredIds)
    {
        PendingTransactions.Remove(Id);
    }
}