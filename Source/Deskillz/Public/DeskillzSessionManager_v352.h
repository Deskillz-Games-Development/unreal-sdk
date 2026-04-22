// Copyright Deskillz Games. All Rights Reserved.
// DeskillzSessionManager_v352.h - v3.5.2 Session Manager
// Path: Source/Deskillz/Public/DeskillzSessionManager_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzSessionManager_v352.generated.h"

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSessionManager_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzSessionManager_v352* Get();

	/** Consume SSO token from launch URL (1.1) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Session")
	void ConsumeSSOToken();

	/** Check for active session - room/tournament/quickplay (1.2) */
	void CheckForActiveSession(const FOnActiveSession& OnResult);

	/** Get cached active session (1.3) */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Session")
	FDeskillzActiveSessionPayload GetActiveSession() const { return CachedSession; }

	/** Whether an active session exists */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Session")
	bool HasActiveSession() const { return CachedSession.bHasActiveSession; }

	/** Enable guest mode (1.6) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Session")
	void EnableGuestMode();

	/** Disable guest mode */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Session")
	void DisableGuestMode();

	/** Check if in guest mode */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Session")
	bool IsGuest() const { return bIsGuest; }

	/** Check if action is allowed in current mode */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Session")
	bool CanPerformAction(const FString& ActionName) const;

	/** Reset on logout */
	void Reset();

	// Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSSOAuthenticated, const FDeskillzAuthUser&, User);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Session")
	FOnSSOAuthenticated OnSSOAuthenticated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionResumed, const FDeskillzActiveSessionPayload&, Session);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Session")
	FOnSessionResumed OnSessionResumed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuestModeActivated);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Session")
	FOnGuestModeActivated OnGuestModeActivated;

private:
	bool bIsGuest = false;
	FDeskillzActiveSessionPayload CachedSession;

	static FString ExtractQueryParam(const FString& Url, const FString& ParamName);
};