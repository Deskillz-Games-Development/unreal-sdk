// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzSocialTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzBuyInManager.generated.h"

/**
 * Buy-in request
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzBuyInRequest
{
    GENERATED_BODY()

    /** Request ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RequestId;

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RoomId;

    /** Amount in dollars */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double Amount = 0.0;

    /** Currency */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    EDeskillzCurrency Currency = EDeskillzCurrency::USDT_BSC;

    /** Points to receive */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    int32 PointsToReceive = 0;

    /** Whether this is a rebuy */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    bool bIsRebuy = false;

    /** When requested */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FDateTime RequestedAt;

    FDeskillzBuyInRequest() = default;
};

/**
 * Buy-in response
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzBuyInResponse
{
    GENERATED_BODY()

    /** Whether successful */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    bool bSuccess = false;

    /** Request ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RequestId;

    /** Transaction ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString TransactionId;

    /** Amount charged */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double AmountCharged = 0.0;

    /** Points received */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    int32 PointsReceived = 0;

    /** New balance */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    int32 NewBalance = 0;

    /** Error message (if failed) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString ErrorMessage;

    /** Error code (if failed) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString ErrorCode;

    FDeskillzBuyInResponse() = default;

    static FDeskillzBuyInResponse Success(const FString& InRequestId, const FString& InTxId, double Amount, int32 Points, int32 Balance)
    {
        FDeskillzBuyInResponse Response;
        Response.bSuccess = true;
        Response.RequestId = InRequestId;
        Response.TransactionId = InTxId;
        Response.AmountCharged = Amount;
        Response.PointsReceived = Points;
        Response.NewBalance = Balance;
        return Response;
    }

    static FDeskillzBuyInResponse Failure(const FString& InRequestId, const FString& Error, const FString& Code = TEXT(""))
    {
        FDeskillzBuyInResponse Response;
        Response.bSuccess = false;
        Response.RequestId = InRequestId;
        Response.ErrorMessage = Error;
        Response.ErrorCode = Code;
        return Response;
    }
};

/**
 * Cash out request
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzCashOutRequest
{
    GENERATED_BODY()

    /** Request ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RequestId;

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RoomId;

    /** Current balance in points */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    int32 BalancePoints = 0;

    /** Balance in dollars */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double BalanceDollars = 0.0;

    /** When requested */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FDateTime RequestedAt;

    FDeskillzCashOutRequest() = default;
};

/**
 * Cash out response
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzCashOutResponse
{
    GENERATED_BODY()

    /** Whether successful */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    bool bSuccess = false;

    /** Request ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString RequestId;

    /** Transaction ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString TransactionId;

    /** Amount returned to wallet */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double AmountReturned = 0.0;

    /** Total buy-in this session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double TotalBuyIn = 0.0;

    /** Net profit/loss */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    double NetResult = 0.0;

    /** Rounds played */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    int32 RoundsPlayed = 0;

    /** Error message (if failed) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|BuyIn")
    FString ErrorMessage;

    FDeskillzCashOutResponse() = default;
};

/**
 * Callback delegates for buy-in operations
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBuyInComplete, const FDeskillzBuyInResponse&, Response);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCashOutComplete, const FDeskillzCashOutResponse&, Response);

/**
 * Deskillz Buy-In Manager
 * 
 * Manages buy-in, rebuy, and cash-out operations for social games.
 * Handles validation, wallet integration, and transaction processing.
 * 
 * Features:
 * - Buy-in validation against room limits
 * - Wallet balance checking
 * - Rebuy tracking and limits
 * - Cash-out processing
 * - Transaction history
 * 
 * Usage:
 * 1. ValidateBuyIn() to check amount
 * 2. ProcessBuyIn() to execute buy-in
 * 3. ProcessCashOut() when leaving
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzBuyInManager : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzBuyInManager();

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * Initialize with room settings
     * @param Settings Room game settings
     * @param InRoomId Room ID
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void Initialize(const FDeskillzSocialGameSettings& Settings, const FString& InRoomId);

    /**
     * Reset manager state
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void Reset();

    // ========================================================================
    // Validation
    // ========================================================================

    /**
     * Validate a buy-in amount
     * @param Amount Amount in dollars
     * @param WalletBalance Player's wallet balance
     * @param bIsRebuy Whether this is a rebuy
     * @return Validation result
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    FDeskillzBuyInValidation ValidateBuyIn(double Amount, double WalletBalance, bool bIsRebuy = false) const;

    /**
     * Check if player can rebuy
     * @param CurrentRebuyCount Player's current rebuy count
     * @return Whether rebuy is allowed
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    bool CanRebuy(int32 CurrentRebuyCount) const;

    /**
     * Get remaining rebuys
     * @param CurrentRebuyCount Current rebuy count
     * @return Remaining allowed rebuys
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    int32 GetRemainingRebuys(int32 CurrentRebuyCount) const;

    // ========================================================================
    // Buy-In Operations
    // ========================================================================

    /**
     * Process a buy-in request
     * @param Amount Amount in dollars
     * @param Currency Currency to use
     * @param Callback Completion callback
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void ProcessBuyIn(double Amount, EDeskillzCurrency Currency, const FOnBuyInComplete& Callback);

    /**
     * Process a rebuy request
     * @param Amount Amount in dollars
     * @param Currency Currency to use
     * @param CurrentRebuyCount Player's current rebuy count
     * @param Callback Completion callback
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void ProcessRebuy(double Amount, EDeskillzCurrency Currency, int32 CurrentRebuyCount, const FOnCashOutComplete& Callback);

    /**
     * Process cash-out
     * @param BalancePoints Current balance in points
     * @param TotalBuyIn Total buy-in this session
     * @param RoundsPlayed Rounds played
     * @param Callback Completion callback
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void ProcessCashOut(int32 BalancePoints, double TotalBuyIn, int32 RoundsPlayed, const FOnCashOutComplete& Callback);

    /**
     * Cancel pending buy-in request
     * @param RequestId Request to cancel
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    void CancelBuyIn(const FString& RequestId);

    // ========================================================================
    // Buy-In Options
    // ========================================================================

    /**
     * Get buy-in options for the room
     * @return Buy-in options
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    FDeskillzBuyInOptions GetBuyInOptions() const;

    /**
     * Get suggested buy-in amounts
     * @param WalletBalance Player's wallet balance
     * @return Array of suggested amounts
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|BuyIn")
    TArray<double> GetSuggestedAmounts(double WalletBalance) const;

    /**
     * Get minimum buy-in in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    double GetMinBuyIn() const;

    /**
     * Get maximum buy-in in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    double GetMaxBuyIn() const;

    /**
     * Get point value
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    double GetPointValue() const { return CurrentSettings.PointValue; }

    // ========================================================================
    // Conversion
    // ========================================================================

    /**
     * Convert dollars to points
     * @param Dollars Amount in dollars
     * @return Points
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    int32 DollarsToPoints(double Dollars) const;

    /**
     * Convert points to dollars
     * @param Points Point amount
     * @return Dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    double PointsToDollars(int32 Points) const;

    // ========================================================================
    // State
    // ========================================================================

    /**
     * Check if a buy-in is in progress
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    bool IsBuyInInProgress() const { return bBuyInInProgress; }

    /**
     * Get pending request
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    FDeskillzBuyInRequest GetPendingRequest() const { return PendingRequest; }

    /**
     * Get current room settings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|BuyIn")
    FDeskillzSocialGameSettings GetSettings() const { return CurrentSettings; }

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current room settings */
    UPROPERTY()
    FDeskillzSocialGameSettings CurrentSettings;

    /** Current room ID */
    FString RoomId;

    /** Whether initialized */
    bool bInitialized = false;

    /** Whether buy-in in progress */
    bool bBuyInInProgress = false;

    /** Pending request */
    UPROPERTY()
    FDeskillzBuyInRequest PendingRequest;

    /** Pending callback */
    FOnBuyInComplete PendingBuyInCallback;

    /** Pending cash out callback */
    FOnCashOutComplete PendingCashOutCallback;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Generate request ID */
    FString GenerateRequestId() const;

    /** Make API request for buy-in */
    void ExecuteBuyInRequest(const FDeskillzBuyInRequest& Request);

    /** Make API request for cash-out */
    void ExecuteCashOutRequest(const FDeskillzCashOutRequest& Request);

    /** Handle buy-in response */
    void HandleBuyInResponse(bool bSuccess, const TSharedPtr<FJsonObject>& ResponseData);

    /** Handle cash-out response */
    void HandleCashOutResponse(bool bSuccess, const TSharedPtr<FJsonObject>& ResponseData);
};