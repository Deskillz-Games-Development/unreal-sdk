// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzTournamentListWidget.generated.h"

class UScrollBox;
class UVerticalBox;
class UTextBlock;
class UButton;
class UImage;
class UBorder;
class UComboBoxString;

/**
 * Tournament filter options
 */
UENUM(BlueprintType)
enum class EDeskillzTournamentFilter : uint8
{
	All,
	Free,
	Paid,
	Featured,
	Ending_Soon
};

/**
 * Tournament sort options
 */
UENUM(BlueprintType)
enum class EDeskillzTournamentSort : uint8
{
	Featured,
	Prize_High,
	Prize_Low,
	Entry_Low,
	Entry_High,
	Players,
	Ending_Soon
};

/** Delegate for tournament selection */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentSelected, const FString&, TournamentId);

/**
 * Tournament List Widget
 * 
 * Displays available tournaments with:
 * - Filtering (Free/Paid/Featured)
 * - Sorting (Prize, Entry, Players)
 * - Pull-to-refresh
 * - Tournament cards with details
 * - Quick-enter functionality
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzTournamentListWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzTournamentListWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Refresh tournament list from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void RefreshTournaments();
	
	/**
	 * Set filter option
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetFilter(EDeskillzTournamentFilter Filter);
	
	/**
	 * Set sort option
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetSort(EDeskillzTournamentSort Sort);
	
	/**
	 * Get currently selected tournament ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	FString GetSelectedTournamentId() const { return SelectedTournamentId; }
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when a tournament is selected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnTournamentSelected OnTournamentSelected;
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings (Set in Blueprint or code)
	// ========================================================================
	
	/** Scroll container for tournament list */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|UI")
	UScrollBox* TournamentScrollBox;
	
	/** Container for tournament cards */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|UI")
	UVerticalBox* TournamentContainer;
	
	/** Title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TitleText;
	
	/** Filter dropdown */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UComboBoxString* FilterComboBox;
	
	/** Sort dropdown */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UComboBoxString* SortComboBox;
	
	/** Refresh button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* RefreshButton;
	
	/** Back button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* BackButton;
	
	/** Loading indicator */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* LoadingIndicator;
	
	/** Empty state text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* EmptyStateText;
	
	/** Balance display text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* BalanceText;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Tournament card widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TSubclassOf<UUserWidget> TournamentCardClass;
	
	/** Max tournaments to display */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	int32 MaxTournamentsToShow = 50;
	
	/** Auto-refresh interval (0 to disable) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float AutoRefreshInterval = 30.0f;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Cached tournament list */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FDeskillzTournament> Tournaments;
	
	/** Currently selected tournament */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FString SelectedTournamentId;
	
	/** Current filter */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	EDeskillzTournamentFilter CurrentFilter = EDeskillzTournamentFilter::All;
	
	/** Current sort */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	EDeskillzTournamentSort CurrentSort = EDeskillzTournamentSort::Featured;
	
	/** Is currently loading */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsLoading = false;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Populate the list with tournament cards */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PopulateTournamentList();
	
	/** Create a tournament card widget */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	UUserWidget* CreateTournamentCard(const FDeskillzTournament& Tournament);
	
	/** Filter and sort tournaments */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	TArray<FDeskillzTournament> GetFilteredTournaments() const;
	
	/** Handle tournament card clicked */
	UFUNCTION()
	void OnTournamentCardClicked(const FString& TournamentId);
	
	/** Handle enter button clicked */
	UFUNCTION()
	void OnEnterTournament(const FString& TournamentId);
	
	/** Handle filter changed */
	UFUNCTION()
	void OnFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	/** Handle sort changed */
	UFUNCTION()
	void OnSortChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	/** Handle refresh clicked */
	UFUNCTION()
	void OnRefreshClicked();
	
	/** Handle back clicked */
	UFUNCTION()
	void OnBackClicked();
	
	/** Called when tournaments loaded */
	void OnTournamentsLoaded(bool bSuccess, const TArray<FDeskillzTournament>& LoadedTournaments);
	
	/** Show loading state */
	void SetLoading(bool bLoading);
	
	/** Update balance display */
	void UpdateBalanceDisplay();
	
	/** Start auto-refresh timer */
	void StartAutoRefresh();
	
	/** Stop auto-refresh timer */
	void StopAutoRefresh();
	
	/** Auto-refresh timer handle */
	FTimerHandle AutoRefreshTimerHandle;
};
