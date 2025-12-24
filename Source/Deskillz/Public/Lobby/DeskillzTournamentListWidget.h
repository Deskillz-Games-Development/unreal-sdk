// Copyright Deskillz Games. All Rights Reserved.
// DeskillzTournamentListWidget_Updated.h - UPDATED for centralized lobby architecture
//
// CHANGES FROM ORIGINAL:
// - Removed in-game tournament browsing
// - Now shows informational message directing to main app
// - Simplified to single-purpose: redirect to deskillz.games

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzTournamentListWidget_Updated.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UBorder;

/**
 * Tournament List Widget (Centralized Lobby Version)
 * 
 * In the centralized lobby architecture, tournament browsing is handled
 * by the main Deskillz website/app. This widget now serves as:
 * 
 * 1. An informational display directing players to deskillz.games
 * 2. A quick-access button to open the main app/website
 * 
 * REMOVED FEATURES (now in main app):
 * - Tournament list display
 * - Filtering and sorting
 * - Entry fee display
 * - Join tournament functionality
 * - Real-time tournament updates
 * 
 * KEPT FEATURES:
 * - Basic styling
 * - Theme support
 * - Open app/website button
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzTournamentListWidget_Updated : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzTournamentListWidget_Updated(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Open the main Deskillz app or website
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void OpenDeskillzApp();
	
	/**
	 * Open specific tournament in main app (via deep link)
	 * @param TournamentId Tournament to open
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void OpenTournament(const FString& TournamentId);
	
	/**
	 * Open tournaments list in main app
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void OpenTournamentsList();
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TitleText;
	
	/** Description text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* DescriptionText;
	
	/** Deskillz logo */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* LogoImage;
	
	/** "Open Deskillz" button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* OpenAppButton;
	
	/** Button text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ButtonText;
	
	/** Close button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* CloseButton;
	
	/** Features list text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* FeaturesText;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Main app website URL */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString WebsiteURL = TEXT("https://deskillz.games");
	
	/** Main app deep link scheme */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString AppScheme = TEXT("deskillz://");
	
	/** Title to display */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString DisplayTitle = TEXT("Browse Tournaments");
	
	/** Description to display */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString DisplayDescription = TEXT("Find tournaments, compete with players worldwide, and win cryptocurrency prizes at deskillz.games");
	
	/** Features list */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString FeaturesListText = TEXT("• Browse available tournaments\n• Pay entry fees in crypto\n• Win BTC, ETH, SOL & more\n• Track your earnings");
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Open URL (deep link or web) */
	void OpenURL(const FString& URL);
	
	/** Handle open app clicked */
	UFUNCTION()
	void OnOpenAppClicked();
	
	/** Handle close clicked */
	UFUNCTION()
	void OnCloseClicked();
};