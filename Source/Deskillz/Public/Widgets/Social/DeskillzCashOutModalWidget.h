// =============================================================================
// Deskillz SDK for Unreal Engine - Cash Out Modal Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzCashOutModalWidget.generated.h"

/**
 * Cash out session summary structure.
 */
USTRUCT(BlueprintType)
struct FDeskillzCashOutSummary
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentBalance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalBuyIn = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetProfitLoss = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RoundsPlayed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RoundsWon = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Currency = TEXT("USDT");
};

/**
 * Modal dialog for cashing out of a social game session.
 * Shows session summary and final balance.
 */
UCLASS()
class DESKILLZ_API UDeskillzCashOutModalWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    // =========================================================================
    // DELEGATES
    // =========================================================================

    /** Called when cash out is confirmed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCashOutConfirmed);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnCashOutConfirmed OnCashOutConfirmed;

    /** Called when cancelled (stay in game) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCashOutCancelled);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnCashOutCancelled OnCashOutCancelled;

    /** Called when cash out completes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCashOutComplete, bool, bSuccess, float, AmountReceived);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnCashOutComplete OnCashOutComplete;

    // =========================================================================
    // UI COMPONENTS
    // =========================================================================

    /** Modal background overlay */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UImage* BackgroundOverlay;

    /** Modal container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UBorder* ModalContainer;

    // ----- Header -----

    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TitleText;

    /** Close button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* CloseButton;

    // ----- Session Summary -----

    /** Summary container background */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UBorder* SummaryContainer;

    /** Total buy-in label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TotalBuyInLabel;

    /** Total buy-in amount */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TotalBuyInText;

    /** Current balance label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* CurrentBalanceLabel;

    /** Current balance amount */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* CurrentBalanceText;

    /** Divider line */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UImage* DividerLine;

    /** Rounds played label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RoundsPlayedLabel;

    /** Rounds played count */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RoundsPlayedText;

    /** Rounds won label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RoundsWonLabel;

    /** Rounds won count */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RoundsWonText;

    // ----- Net Result Section -----

    /** Net result container (colored based on profit/loss) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UBorder* NetResultContainer;

    /** Net result label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* NetResultLabel;

    /** Net result amount (large, colored) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* NetResultText;

    // ----- Info Section -----

    /** Information text about withdrawal */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* InfoText;

    // ----- Buttons -----

    /** Stay in game button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* StayButton;

    /** Stay button text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* StayButtonText;

    /** Cash out button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* CashOutButton;

    /** Cash out button text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* CashOutButtonText;

    // ----- Loading -----

    /** Loading indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UCircularThrobber* LoadingIndicator;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    /** Color for profit result background */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor ProfitBackgroundColor = FLinearColor(0.1f, 0.25f, 0.15f, 1.0f);

    /** Color for loss result background */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor LossBackgroundColor = FLinearColor(0.25f, 0.1f, 0.1f, 1.0f);

    /** Color for profit text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor ProfitTextColor = FLinearColor(0.2f, 0.8f, 0.4f, 1.0f);

    /** Color for loss text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor LossTextColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);

    // =========================================================================
    // PUBLIC METHODS
    // =========================================================================

    /** Show the cash out modal with session summary */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void ShowModal(const FDeskillzCashOutSummary& Summary);

    /** Hide the modal */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void HideModal();

    /** Called when cash out operation completes */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void OnOperationComplete(bool bSuccess, float AmountReceived = 0.0f);

    /** Get the current session summary */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    FDeskillzCashOutSummary GetSessionSummary() const { return SessionSummary; }

    /** Check if user is in profit */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    bool IsInProfit() const { return SessionSummary.NetProfitLoss >= 0.0f; }

    /** Get win rate percentage */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    float GetWinRate() const;

protected:
    virtual void NativeConstruct() override;

    // =========================================================================
    // UI EVENT HANDLERS
    // =========================================================================

    UFUNCTION()
    void OnCashOutClicked();

    UFUNCTION()
    void OnStayClicked();

    UFUNCTION()
    void OnCloseClicked();

    UFUNCTION()
    void OnOverlayClicked();

    // =========================================================================
    // HELPERS
    // =========================================================================

    /** Update UI from session summary */
    void UpdateUIFromSummary();

    /** Update net result display colors */
    void UpdateNetResultColors();

    /** Set processing state */
    void SetProcessing(bool bProcessing);

    /** Format currency amount */
    FString FormatCurrency(float Amount) const;

    /** Format profit/loss with sign */
    FString FormatProfitLoss(float Amount) const;

private:
    /** Current session summary */
    FDeskillzCashOutSummary SessionSummary;

    /** Whether processing cash out */
    bool bIsProcessing = false;
};