// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzTurnTimerWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UBorder;
class UMaterialInstanceDynamic;

/**
 * Timer display style
 */
UENUM(BlueprintType)
enum class EDeskillzTimerStyle : uint8
{
    Circular    UMETA(DisplayName = "Circular"),
    Bar         UMETA(DisplayName = "Progress Bar"),
    Text        UMETA(DisplayName = "Text Only"),
    Compact     UMETA(DisplayName = "Compact")
};

/**
 * Timer state
 */
UENUM(BlueprintType)
enum class EDeskillzTimerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Running     UMETA(DisplayName = "Running"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Expired     UMETA(DisplayName = "Expired"),
    Paused      UMETA(DisplayName = "Paused")
};

/**
 * Deskillz Turn Timer Widget
 * 
 * Displays turn countdown timer with:
 * - Circular or bar progress
 * - Warning/critical states with colors
 * - Optional sound effects
 * - Player name display
 * - Pause state handling
 * 
 * Usage:
 * 1. Add to game UI
 * 2. Call StartTimer() when turn begins
 * 3. Handle OnTimerExpired delegate
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzTurnTimerWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzTurnTimerWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========================================================================
    // Timer Control
    // ========================================================================

    /**
     * Start the timer
     * @param DurationSeconds Total duration
     * @param PlayerName Name of player whose turn it is
     * @param bIsLocalPlayer Whether this is the local player's turn
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void StartTimer(float DurationSeconds, const FString& PlayerName = TEXT(""), bool bIsLocalPlayer = false);

    /**
     * Stop the timer
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void StopTimer();

    /**
     * Pause the timer
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void PauseTimer();

    /**
     * Resume the timer
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void ResumeTimer();

    /**
     * Set remaining time directly
     * @param Seconds Remaining seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void SetRemainingTime(float Seconds);

    /**
     * Get remaining time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    float GetRemainingTime() const { return RemainingTime; }

    /**
     * Get total duration
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    float GetTotalDuration() const { return TotalDuration; }

    /**
     * Get progress (0-1)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    float GetProgress() const;

    /**
     * Get current state
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    EDeskillzTimerState GetTimerState() const { return CurrentState; }

    /**
     * Check if timer is running
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    bool IsRunning() const { return bIsRunning && !bIsPaused; }

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Timer display style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    EDeskillzTimerStyle TimerStyle = EDeskillzTimerStyle::Circular;

    /** Warning threshold (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    float WarningThreshold = 10.0f;

    /** Critical threshold (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    float CriticalThreshold = 5.0f;

    /** Normal color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    FLinearColor NormalColor = FLinearColor(0.2f, 0.8f, 0.2f);

    /** Warning color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    FLinearColor WarningColor = FLinearColor(1.0f, 0.8f, 0.0f);

    /** Critical color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    FLinearColor CriticalColor = FLinearColor(1.0f, 0.2f, 0.2f);

    /** Play tick sounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bPlayTickSounds = true;

    /** Tick sound interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    float TickSoundInterval = 1.0f;

    /** Show player name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bShowPlayerName = true;

    /** Pulse animation on critical */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bPulseOnCritical = true;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when timer expires */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerExpired);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnTimerExpired OnTimerExpired;

    /** Called when entering warning state */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerWarning);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnTimerWarning OnTimerWarning;

    /** Called when entering critical state */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerCritical);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnTimerCritical OnTimerCritical;

    /** Called every tick with remaining time */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerTick, float, RemainingSeconds);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnTimerTick OnTimerTick;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Timer container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UBorder* TimerContainer;

    /** Circular progress image */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UImage* CircularProgressImage;

    /** Progress bar (for bar style) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UProgressBar* ProgressBar;

    /** Time text (seconds remaining) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* TimeText;

    /** Player name text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PlayerNameText;

    /** "Your Turn" indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* YourTurnText;

    /** Paused indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PausedText;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Total duration */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    float TotalDuration = 0.0f;

    /** Remaining time */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    float RemainingTime = 0.0f;

    /** Is timer running */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    bool bIsRunning = false;

    /** Is timer paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    bool bIsPaused = false;

    /** Is local player's turn */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    bool bIsLocalPlayerTurn = false;

    /** Current state */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    EDeskillzTimerState CurrentState = EDeskillzTimerState::Idle;

    /** Dynamic material for circular progress */
    UPROPERTY()
    UMaterialInstanceDynamic* CircularProgressMaterial;

    /** Last tick sound time */
    float LastTickSoundTime = 0.0f;

    /** Pulse animation alpha */
    float PulseAlpha = 0.0f;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Update visual display */
    void UpdateDisplay();

    /** Update timer state */
    void UpdateTimerState();

    /** Update colors based on state */
    void UpdateColors();

    /** Play tick sound */
    void PlayTickSound();

    /** Update pulse animation */
    void UpdatePulseAnimation(float DeltaTime);

    /** Get color for current state */
    FLinearColor GetCurrentColor() const;

    /** Format time as string */
    FString FormatTime(float Seconds) const;
};