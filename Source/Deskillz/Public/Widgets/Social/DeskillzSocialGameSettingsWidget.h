// =============================================================================
// Deskillz SDK for Unreal Engine - Social Game Settings Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "DeskillzSocialGameSettingsWidget.generated.h"

/**
 * Social game settings configuration structure.
 */
USTRUCT(BlueprintType)
struct FDeskillzSocialGameSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PointValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RakePercent = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RakeCap = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTurnTimerEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TurnTimerSeconds = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPauseEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPausesPerPlayer = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPauseDurationMinutes = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PauseCooldownMinutes = 15;
};

/**
 * Widget for configuring social game room settings.
 * Allows setting point value, rake, timer options, and pause rules.
 */
UCLASS()
class DESKILLZ_API UDeskillzSocialGameSettingsWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    // =========================================================================
    // DELEGATES
    // =========================================================================

    /** Called when settings are confirmed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsConfirmed, const FDeskillzSocialGameSettings&, Settings);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnSettingsConfirmed OnSettingsConfirmed;

    /** Called when cancelled */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsCancelled);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnSettingsCancelled OnSettingsCancelled;

    /** Called when settings change (for preview) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, const FDeskillzSocialGameSettings&, Settings);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
    FOnSettingsChanged OnSettingsChanged;

    // =========================================================================
    // UI COMPONENTS
    // =========================================================================

    /** Modal background overlay */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UImage* BackgroundOverlay;

    /** Modal container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UBorder* ModalContainer;

    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TitleText;

    /** Close button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* CloseButton;

    // ----- Point Value Section -----

    /** Point value slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    USlider* PointValueSlider;

    /** Point value input field */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UEditableTextBox* PointValueInput;

    /** Minimum buy-in display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* MinBuyInText;

    // ----- Rake Section -----

    /** Rake percentage slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    USlider* RakePercentSlider;

    /** Rake percentage display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* RakePercentText;

    /** Rake cap input */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UEditableTextBox* RakeCapInput;

    // ----- Turn Timer Section -----

    /** Turn timer enabled checkbox */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UCheckBox* TurnTimerCheckbox;

    /** Turn timer duration slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    USlider* TurnTimerSlider;

    /** Turn timer duration display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* TurnTimerText;

    // ----- Pause Section -----

    /** Pause enabled checkbox */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UCheckBox* PauseEnabledCheckbox;

    /** Max pauses per player slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    USlider* MaxPausesSlider;

    /** Max pauses display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* MaxPausesText;

    /** Pause duration slider */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    USlider* PauseDurationSlider;

    /** Pause duration display */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* PauseDurationText;

    // ----- Revenue Preview -----

    /** Host share percentage preview */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* HostSharePreviewText;

    /** Estimated earnings preview */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* EstimatedEarningsText;

    // ----- Buttons -----

    /** Cancel button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* CancelButton;

    /** Confirm/Create button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UButton* ConfirmButton;

    /** Confirm button text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* ConfirmButtonText;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    /** Minimum point value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinPointValue = 0.1f;

    /** Maximum point value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxPointValue = 100.0f;

    /** Default point value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DefaultPointValue = 1.0f;

    /** Minimum rake percentage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinRakePercent = 1.0f;

    /** Maximum rake percentage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxRakePercent = 10.0f;

    /** Minimum turn timer seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MinTurnTimerSeconds = 10;

    /** Maximum turn timer seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxTurnTimerSeconds = 120;

    // =========================================================================
    // PUBLIC METHODS
    // =========================================================================

    /** Show the settings modal */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void ShowModal();

    /** Hide the settings modal */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void HideModal();

    /** Set initial settings values */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void SetSettings(const FDeskillzSocialGameSettings& Settings);

    /** Get current settings */
    UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
    FDeskillzSocialGameSettings GetCurrentSettings() const;

    /** Set host tier for revenue preview calculation */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void SetHostTier(int32 TierLevel);

    /** Update the revenue preview */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
    void UpdateRevenuePreview();

protected:
    virtual void NativeConstruct() override;

    // =========================================================================
    // UI EVENT HANDLERS
    // =========================================================================

    UFUNCTION()
    void OnPointValueSliderChanged(float Value);

    UFUNCTION()
    void OnPointValueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    UFUNCTION()
    void OnRakeSliderChanged(float Value);

    UFUNCTION()
    void OnRakeCapInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    UFUNCTION()
    void OnTurnTimerCheckboxChanged(bool bIsChecked);

    UFUNCTION()
    void OnTurnTimerSliderChanged(float Value);

    UFUNCTION()
    void OnPauseCheckboxChanged(bool bIsChecked);

    UFUNCTION()
    void OnMaxPausesSliderChanged(float Value);

    UFUNCTION()
    void OnPauseDurationSliderChanged(float Value);

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnCloseClicked();

    // =========================================================================
    // HELPERS
    // =========================================================================

    /** Update UI from current settings */
    void UpdateUIFromSettings();

    /** Update minimum buy-in display */
    void UpdateMinBuyInDisplay();

    /** Calculate estimated host earnings */
    float CalculateEstimatedEarnings() const;

    /** Get host share percentage for current tier */
    float GetHostSharePercent() const;

private:
    /** Current settings being edited */
    FDeskillzSocialGameSettings CurrentSettings;

    /** Current host tier level (0-5) */
    int32 HostTierLevel = 0;
};