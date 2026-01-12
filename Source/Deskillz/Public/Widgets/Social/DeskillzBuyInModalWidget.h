// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Social/DeskillzSocialTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzBuyInModalWidget.generated.h"

class UTextBlock;
class UButton;
class USlider;
class UEditableTextBox;
class UHorizontalBox;
class UVerticalBox;
class UBorder;
class UImage;
class UComboBoxString;

/**
 * Deskillz Buy-In Modal Widget
 * 
 * Modal for buying into social game sessions:
 * - Amount input (slider + text)
 * - Currency selection
 * - Wallet balance display
 * - Points preview
 * - Confirmation/cancel buttons
 * 
 * Usage:
 * 1. Show modal via ShowModal()
 * 2. User selects amount and currency
 * 3. Handle OnBuyInConfirmed delegate
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzBuyInModalWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzBuyInModalWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Modal Control
    // ========================================================================

    /**
     * Show the buy-in modal
     * @param Options Buy-in options
     * @param WalletBalance Current wallet balance
     * @param Currency Default currency
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void ShowModal(const FDeskillzBuyInOptions& Options, double WalletBalance, EDeskillzCurrency Currency = EDeskillzCurrency::USDT_BSC);

    /**
     * Hide the modal
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void HideModal();

    /**
     * Set buy-in options
     * @param Options Buy-in configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void SetBuyInOptions(const FDeskillzBuyInOptions& Options);

    /**
     * Set wallet balance
     * @param Balance Current balance
     * @param Currency Currency type
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void SetWalletBalance(double Balance, EDeskillzCurrency Currency);

    /**
     * Set the buy-in amount
     * @param Amount Amount in dollars
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void SetAmount(double Amount);

    /**
     * Get current amount
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    double GetAmount() const { return CurrentAmount; }

    /**
     * Get selected currency
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    EDeskillzCurrency GetSelectedCurrency() const { return SelectedCurrency; }

    /**
     * Validate current amount
     * @return Validation result
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    FDeskillzBuyInValidation ValidateAmount() const;

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Show suggested amounts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bShowSuggestedAmounts = true;

    /** Show currency selector */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bShowCurrencySelector = true;

    /** Show points preview */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bShowPointsPreview = true;

    /** Animation duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    float AnimationDuration = 0.3f;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when buy-in is confirmed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuyInConfirmed, double, Amount, EDeskillzCurrency, Currency);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnBuyInConfirmed OnBuyInConfirmed;

    /** Called when modal is cancelled */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuyInCancelled);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnBuyInCancelled OnBuyInCancelled;

    /** Called when amount changes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmountChanged, double, NewAmount);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnAmountChanged OnAmountChanged;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Modal background */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UBorder* ModalBackground;

    /** Modal container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UBorder* ModalContainer;

    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* TitleText;

    /** Close button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* CloseButton;

    /** Wallet balance text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* WalletBalanceText;

    /** Amount slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    USlider* AmountSlider;

    /** Amount input text box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UEditableTextBox* AmountInput;

    /** Min amount text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* MinAmountText;

    /** Max amount text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* MaxAmountText;

    /** Suggested amounts container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UHorizontalBox* SuggestedAmountsContainer;

    /** Points preview text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PointsPreviewText;

    /** Currency selector */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UComboBoxString* CurrencySelector;

    /** Error text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* ErrorText;

    /** Confirm button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* ConfirmButton;

    /** Cancel button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* CancelButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current buy-in options */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    FDeskillzBuyInOptions CurrentOptions;

    /** Current amount */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    double CurrentAmount = 0.0;

    /** Wallet balance */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    double WalletBalance = 0.0;

    /** Selected currency */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    EDeskillzCurrency SelectedCurrency = EDeskillzCurrency::USDT_BSC;

    /** Is modal visible */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    bool bIsVisible = false;

    /** Suggested amount buttons */
    UPROPERTY()
    TArray<UButton*> SuggestedAmountButtons;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Update all displays */
    void UpdateDisplay();

    /** Update points preview */
    void UpdatePointsPreview();

    /** Update error display */
    void UpdateErrorDisplay();

    /** Update confirm button state */
    void UpdateConfirmButtonState();

    /** Create suggested amount buttons */
    void CreateSuggestedAmountButtons();

    /** Convert amount to slider value */
    float AmountToSliderValue(double Amount) const;

    /** Convert slider value to amount */
    double SliderValueToAmount(float Value) const;

    // Event handlers
    UFUNCTION()
    void OnSliderValueChanged(float Value);

    UFUNCTION()
    void OnAmountInputChanged(const FText& Text);

    UFUNCTION()
    void OnAmountInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    UFUNCTION()
    void OnCurrencyChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnSuggestedAmountClicked(int32 Index);

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnCloseClicked();

    UFUNCTION()
    void OnBackgroundClicked();
};