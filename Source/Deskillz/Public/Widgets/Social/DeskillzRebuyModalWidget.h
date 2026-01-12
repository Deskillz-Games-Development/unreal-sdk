// =============================================================================
// Deskillz SDK for Unreal Engine - Rebuy Modal Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "DeskillzRebuyModalWidget.generated.h"

/**
 * Session summary for rebuy display.
 */
USTRUCT(BlueprintType)
struct FDeskillzRebuySessionSummary
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalBuyIn = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RoundsPlayed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetProfitLoss = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PointValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalletBalance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinRebuy = 50.0f;
};

/**
 * Modal dialog for rebuy after busting in social games.
 * Shows session stats and rebuy options with decision timer.
 */
UCLASS()
class DESKILLZ_API UDeskillzRebuyModalWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    // =========================================================================
    // DELEGATES
    // =========================================================================

    /** Called when rebuy is confirmed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRebuyConfirmed, float, Amount, const FString&, Currency);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnRebuyConfirmed OnRebuyConfirmed;

    /** Called when player chooses to leave */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeaveGameClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnLeaveGameClicked OnLeaveGameClicked;

    /** Called when decision timer expires */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDecisionTimerExpired);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnDecisionTimerExpired OnDecisionTimerExpired;

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

    /** Warning icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UImage* WarningIcon;

    /** Title text ("You're Out!") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TitleText;

    /** Subtitle text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* SubtitleText;

    /** Decision timer display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TimerText;

    // ----- Session Summary -----

    /** Summary container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UBorder* SummaryContainer;

    /** Total buy-in amount */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TotalBuyInText;

    /** Rounds played count */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RoundsPlayedText;

    /** Net profit/loss amount */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* NetProfitLossText;

    // ----- Rebuy Section -----

    /** Wallet balance display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* WalletBalanceText;

    /** Minimum rebuy display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* MinRebuyText;

    /** Rebuy amount input */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UEditableTextBox* AmountInput;

    /** Currency dropdown */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UComboBoxString* CurrencyDropdown;

    /** Preset button: Minimum */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* PresetMinButton;

    /** Preset button: 100x */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* Preset100xButton;

    /** Preset button: 200x */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* Preset200xButton;

    /** Preset button texts */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* PresetMinText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Preset100xText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Preset200xText;

    // ----- Buttons -----

    /** Leave game button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* LeaveButton;

    /** Leave button text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* LeaveButtonText;

    /** Rebuy button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* RebuyButton;

    /** Rebuy button text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RebuyButtonText;

    // ----- Loading -----

    /** Loading indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UCircularThrobber* LoadingIndicator;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    /** Decision time limit in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DecisionTimeLimit = 60.0f;

    /** Warning threshold for timer (changes color) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TimerWarningThreshold = 10.0f;

    /** Auto-leave when timer expires */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoLeaveOnTimerExpiry = true;

    /** Available currencies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FString> AvailableCurrencies = { TEXT("USDT"), TEXT("USDC"), TEXT("BNB") };

    // =========================================================================
    // PUBLIC METHODS
    // =========================================================================

    /** Show the rebuy modal with session summary */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void ShowModal(const FDeskillzRebuySessionSummary& Summary);

    /** Hide the modal */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void HideModal();

    /** Called when rebuy operation completes */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void OnRebuyComplete(bool bSuccess, const FString& ErrorMessage = TEXT(""));

    /** Get the selected rebuy amount */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    float GetSelectedAmount() const { return SelectedAmount; }

    /** Get the selected currency */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    FString GetSelectedCurrency() const { return SelectedCurrency; }

    /** Get remaining decision time */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    float GetRemainingTime() const { return RemainingTime; }

    /** Pause the decision timer */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void PauseTimer();

    /** Resume the decision timer */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void ResumeTimer();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // =========================================================================
    // UI EVENT HANDLERS
    // =========================================================================

    UFUNCTION()
    void OnAmountInputChanged(const FText& Text);

    UFUNCTION()
    void OnCurrencySelected(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnPresetMinClicked();

    UFUNCTION()
    void OnPreset100xClicked();

    UFUNCTION()
    void OnPreset200xClicked();

    UFUNCTION()
    void OnRebuyClicked();

    UFUNCTION()
    void OnLeaveClicked();

    // =========================================================================
    // HELPERS
    // =========================================================================

    /** Update UI from session summary */
    void UpdateUIFromSummary();

    /** Update button states based on current selection */
    void UpdateButtonStates();

    /** Update timer display */
    void UpdateTimerDisplay();

    /** Set the selected amount and update UI */
    void SetSelectedAmount(float Amount);

    /** Validate current selection */
    bool ValidateSelection() const;

    /** Set processing state */
    void SetProcessing(bool bProcessing);

    /** Handle timer expiry */
    void HandleTimerExpired();

private:
    /** Current session summary */
    FDeskillzRebuySessionSummary SessionSummary;

    /** Selected rebuy amount */
    float SelectedAmount = 0.0f;

    /** Selected currency */
    FString SelectedCurrency = TEXT("USDT");

    /** Remaining decision time */
    float RemainingTime = 60.0f;

    /** Whether timer is running */
    bool bTimerActive = false;

    /** Whether processing rebuy */
    bool bIsProcessing = false;

    /** Preset amounts calculated from point value */
    float PresetMin = 50.0f;
    float Preset100x = 100.0f;
    float Preset200x = 200.0f;
};