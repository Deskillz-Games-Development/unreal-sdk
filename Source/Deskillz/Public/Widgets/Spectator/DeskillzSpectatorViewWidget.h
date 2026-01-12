// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Social/DeskillzSocialTypes.h"
#include "DeskillzPauseRequestWidget.generated.h"

class UTextBlock;
class UButton;
class UProgressBar;
class UHorizontalBox;
class UVerticalBox;
class UBorder;
class UImage;

/**
 * Deskillz Pause Request Widget
 * 
 * Displays pause request voting UI:
 * - Requester info
 * - Voting progress
 * - Yes/No buttons
 * - Countdown timer
 * - Vote tally display
 * 
 * Also displays active pause state with countdown.
 * 
 * Usage:
 * 1. Call ShowPauseRequest() when vote starts
 * 2. Handle OnVoteSubmitted delegate
 * 3. Call ShowActivePause() when paused
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzPauseRequestWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzPauseRequestWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========================================================================
    // Pause Request Voting
    // ========================================================================

    /**
     * Show pause request voting UI
     * @param Request Pause request data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void ShowPauseRequest(const FDeskillzPauseRequest& Request);

    /**
     * Update vote status
     * @param Request Updated request with votes
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void UpdateVoteStatus(const FDeskillzPauseRequest& Request);

    /**
     * Hide the pause request UI
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void HidePauseRequest();

    /**
     * Submit vote
     * @param bApprove Yes or No
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void SubmitVote(bool bApprove);

    /**
     * Check if can vote (haven't voted yet)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    bool CanVote() const { return bCanVote; }

    // ========================================================================
    // Active Pause Display
    // ========================================================================

    /**
     * Show active pause state
     * @param Pause Active pause data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void ShowActivePause(const FDeskillzActivePause& Pause);

    /**
     * Update active pause countdown
     * @param RemainingSeconds Seconds remaining
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void UpdatePauseCountdown(float RemainingSeconds);

    /**
     * Hide active pause display
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social|UI")
    void HideActivePause();

    /**
     * Check if showing pause request
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    bool IsShowingPauseRequest() const { return bShowingRequest; }

    /**
     * Check if showing active pause
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|UI")
    bool IsShowingActivePause() const { return bShowingActivePause; }

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Auto-hide after vote */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bAutoHideAfterVote = false;

    /** Show vote breakdown */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    bool bShowVoteBreakdown = true;

    /** Warning time threshold (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Social|UI")
    float WarningThreshold = 60.0f;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when vote is submitted */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteSubmitted, bool, bApproved);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnVoteSubmitted OnVoteSubmitted;

    /** Called when voting period expires */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVotingExpired);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnVotingExpired OnVotingExpired;

    /** Called when pause ends */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseEnded);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|UI")
    FOnPauseEnded OnPauseEnded;

protected:
    // ========================================================================
    // UI Components - Pause Request
    // ========================================================================

    /** Request container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UBorder* RequestContainer;

    /** Requester avatar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UImage* RequesterAvatar;

    /** Requester name text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* RequesterNameText;

    /** Request message text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* RequestMessageText;

    /** Voting countdown text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* VotingCountdownText;

    /** Voting progress bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UProgressBar* VotingProgressBar;

    /** Yes vote count text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* YesVoteCountText;

    /** No vote count text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* NoVoteCountText;

    /** Votes needed text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* VotesNeededText;

    /** Vote yes button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* VoteYesButton;

    /** Vote no button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* VoteNoButton;

    /** Your vote indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* YourVoteText;

    // ========================================================================
    // UI Components - Active Pause
    // ========================================================================

    /** Active pause container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UBorder* ActivePauseContainer;

    /** Pause icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UImage* PauseIcon;

    /** Pause title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PauseTitleText;

    /** Pause reason text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PauseReasonText;

    /** Pause countdown text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UTextBlock* PauseCountdownText;

    /** Pause progress bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UProgressBar* PauseProgressBar;

    /** Resume button (host only) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Social|UI")
    UButton* ResumeButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current pause request */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    FDeskillzPauseRequest CurrentRequest;

    /** Current active pause */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social|UI")
    FDeskillzActivePause CurrentPause;

    /** Is showing request */
    bool bShowingRequest = false;

    /** Is showing active pause */
    bool bShowingActivePause = false;

    /** Can vote */
    bool bCanVote = true;

    /** Has voted */
    bool bHasVoted = false;

    /** Local player's vote */
    bool bLocalVote = false;

    /** Voting deadline */
    FDateTime VotingDeadline;

    /** Pause end time */
    FDateTime PauseEndTime;

    /** Pause total duration */
    float PauseTotalDuration = 0.0f;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Update voting display */
    void UpdateVotingDisplay();

    /** Update active pause display */
    void UpdateActivePauseDisplay();

    /** Update voting countdown */
    void UpdateVotingCountdown();

    /** Update button states */
    void UpdateButtonStates();

    /** Format countdown time */
    FString FormatCountdown(float Seconds) const;

    /** Load requester avatar */
    void LoadRequesterAvatar(const FString& Url);

    // Event handlers
    UFUNCTION()
    void OnVoteYesClicked();

    UFUNCTION()
    void OnVoteNoClicked();

    UFUNCTION()
    void OnResumeClicked();
};