// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzLeaderboardWidget.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UVerticalBox;
class UComboBoxString;

/**
 * Leaderboard time filter
 */
UENUM(BlueprintType)
enum class EDeskillzLeaderboardPeriod : uint8
{
	Today,
	Week,
	Month,
	AllTime
};

/**
 * Leaderboard entry data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLeaderboardEntry
{
	GENERATED_BODY()
	
	/** Player rank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Rank = 0;
	
	/** Player ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString PlayerId;
	
	/** Player username */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString Username;
	
	/** Player avatar URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString AvatarUrl;
	
	/** Score/points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 Score = 0;
	
	/** Wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Wins = 0;
	
	/** Total matches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 TotalMatches = 0;
	
	/** Win rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float WinRate = 0.0f;
	
	/** Prize earnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	double Earnings = 0.0;
	
	/** Is current player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bIsCurrentPlayer = false;
};

/** Delegate for leaderboard actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeaderboardEntrySelected, const FDeskillzLeaderboardEntry&, Entry);

/**
 * Leaderboard Widget
 * 
 * Displays rankings with:
 * - Global/Tournament leaderboards
 * - Time period filters
 * - Top players highlight
 * - Current player position
 * - Stats display
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzLeaderboardWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzLeaderboardWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Set tournament ID (empty for global)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetTournamentId(const FString& TournamentId);
	
	/**
	 * Refresh leaderboard data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void RefreshLeaderboard();
	
	/**
	 * Set time period filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetPeriod(EDeskillzLeaderboardPeriod Period);
	
	/**
	 * Get current player rank
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	int32 GetCurrentPlayerRank() const;
	
	/**
	 * Scroll to current player
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ScrollToCurrentPlayer();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when a leaderboard entry is selected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnLeaderboardEntrySelected OnEntrySelected;
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TitleText;
	
	/** Period dropdown */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UComboBoxString* PeriodComboBox;
	
	/** Scroll container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UScrollBox* LeaderboardScrollBox;
	
	/** Entries container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UVerticalBox* EntriesContainer;
	
	/** Top 3 container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* TopThreeContainer;
	
	/** First place widget */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* FirstPlaceWidget;
	
	/** Second place widget */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* SecondPlaceWidget;
	
	/** Third place widget */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* ThirdPlaceWidget;
	
	/** Current player rank text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* CurrentPlayerRankText;
	
	/** Back button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* BackButton;
	
	/** Refresh button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* RefreshButton;
	
	/** Loading indicator */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* LoadingIndicator;
	
	/** Empty state text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* EmptyStateText;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Entry row widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TSubclassOf<UUserWidget> EntryRowClass;
	
	/** Max entries to display */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	int32 MaxEntries = 100;
	
	/** Show top 3 separately */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	bool bShowTopThreeSeparately = true;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Current tournament ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FString CurrentTournamentId;
	
	/** Current period */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	EDeskillzLeaderboardPeriod CurrentPeriod = EDeskillzLeaderboardPeriod::AllTime;
	
	/** Leaderboard entries */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FDeskillzLeaderboardEntry> Entries;
	
	/** Current player entry */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FDeskillzLeaderboardEntry CurrentPlayerEntry;
	
	/** Is loading */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsLoading = false;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Populate leaderboard list */
	void PopulateLeaderboard();
	
	/** Create entry row widget */
	UUserWidget* CreateEntryRow(const FDeskillzLeaderboardEntry& Entry);
	
	/** Update top 3 display */
	void UpdateTopThree();
	
	/** Update current player display */
	void UpdateCurrentPlayerDisplay();
	
	/** Set loading state */
	void SetLoading(bool bLoading);
	
	/** Called when data loaded */
	void OnLeaderboardLoaded(bool bSuccess, const TArray<FDeskillzLeaderboardEntry>& LoadedEntries);
	
	/** Handle period changed */
	UFUNCTION()
	void OnPeriodChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	/** Handle entry clicked */
	UFUNCTION()
	void OnEntryClicked(const FDeskillzLeaderboardEntry& Entry);
	
	/** Handle back clicked */
	UFUNCTION()
	void OnBackClicked();
	
	/** Handle refresh clicked */
	UFUNCTION()
	void OnRefreshClicked();
	
	/** Get rank suffix (st, nd, rd, th) */
	static FString GetRankSuffix(int32 Rank);
	
	/** Get rank color */
	FLinearColor GetRankColor(int32 Rank) const;
};
