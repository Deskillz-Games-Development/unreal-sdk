// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzRoomPlayerCard.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UBorder;

/** Delegate for kick action */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerKickRequestedDelegate, const FString&, PlayerId);

/**
 * Individual player card widget for the room lobby.
 * Displays player info, ready status, and host controls.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzRoomPlayerCard : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzRoomPlayerCard(const FObjectInitializer& ObjectInitializer);

	// =========================================================================
	// Widget Lifecycle
	// =========================================================================

	virtual void NativeConstruct() override;

	// =========================================================================
	// Public Methods
	// =========================================================================

	/**
	 * Set the player data to display
	 * @param Player Player data
	 * @param bCanKick Whether kick button should be shown
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void SetPlayer(const FRoomPlayer& Player, bool bCanKick = false);

	/**
	 * Update the ready status
	 * @param bReady New ready status
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void SetReady(bool bReady);

	/**
	 * Get the current player data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	FRoomPlayer GetPlayer() const { return CurrentPlayer; }

	/**
	 * Get the player ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	FString GetPlayerId() const { return CurrentPlayer.Id; }

	/**
	 * Check if this card represents the current user
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	bool IsCurrentUser() const { return CurrentPlayer.bIsCurrentUser; }

	// =========================================================================
	// Events
	// =========================================================================

	/** Called when kick button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnPlayerKickRequestedDelegate OnKickClicked;

protected:
	// =========================================================================
	// UI Components
	// =========================================================================

	/** Card background */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UBorder* CardBackground;

	/** Ready indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UBorder* ReadyIndicator;

	/** Avatar image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UImage* AvatarImage;

	/** Username text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* UsernameText;

	/** Host badge */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UBorder* HostBadge;

	/** Host badge text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* HostBadgeText;

	/** Status text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* StatusText;

	/** Kick button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* KickButton;

	// =========================================================================
	// Settings
	// =========================================================================

	/** Ready color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	FLinearColor ReadyColor;

	/** Not ready color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	FLinearColor NotReadyColor;

	/** Current user highlight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	FLinearColor CurrentUserBackgroundColor;

	/** Default background color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	FLinearColor DefaultBackgroundColor;

	// =========================================================================
	// State
	// =========================================================================

	/** Current player data */
	UPROPERTY()
	FRoomPlayer CurrentPlayer;

	/** Whether kick is allowed */
	bool bCanKickPlayer;

	// =========================================================================
	// Internal Methods
	// =========================================================================

	/** Initialize UI bindings */
	void InitializeUI();

	/** Update all display elements */
	void UpdateDisplay();

	/** Update ready status display */
	void UpdateReadyStatus();

	/** Load avatar from URL */
	void LoadAvatar();

	/** Generate avatar color from username */
	FLinearColor GetAvatarColor(const FString& Username) const;

	// =========================================================================
	// UI Callbacks
	// =========================================================================

	UFUNCTION()
	void OnKickButtonClicked();
};