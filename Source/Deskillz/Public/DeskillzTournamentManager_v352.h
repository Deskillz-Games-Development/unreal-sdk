// Copyright Deskillz Games. All Rights Reserved.
// DeskillzTournamentManager_v352.h - v3.5.2 Tournament Manager
// Path: Source/Deskillz/Public/DeskillzTournamentManager_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzTournamentManager_v352.generated.h"

/**
 * Tournament Manager (v3.5.2)
 *
 * Handles tournament registration, check-in, bracket schedule,
 * and table assignments. All methods are BlueprintCallable.
 *
 * Usage (C++):
 *   auto* TM = UDeskillzTournamentManager_v352::Get();
 *   TM->Register("tournament-id",
 *       FOnTournamentRegistration::CreateLambda([](const FDeskillzTournamentRegistration& Reg) { ... }),
 *       FOnDeskillzApiError::CreateLambda([](const FDeskillzError& Err) { ... })
 *   );
 *
 * Usage (Blueprint):
 *   Use the BlueprintCallable nodes and bind to delegates.
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzTournamentManager_v352 : public UObject
{
	GENERATED_BODY()

public:
	/** Get singleton instance */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournament", meta = (DisplayName = "Get Tournament Manager"))
	static UDeskillzTournamentManager_v352* Get();

	// ========================================================================
	// TOURNAMENT LISTING (3.1)
	// ========================================================================

	/** Get available tournaments with optional filters */
	void GetTournaments(
		const TMap<FString, FString>& Filters,
		const FOnTournamentListings& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Get active tournaments for a game */
	void GetActiveTournaments(
		const FString& GameId,
		const FOnTournamentListings& OnSuccess,
		const FOnDeskillzApiError& OnError);

	// ========================================================================
	// REGISTRATION FLOW (3.2 - 3.5)
	// ========================================================================

	/** Register for a tournament */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournament")
	void Register(
		const FString& TournamentId,
		const FOnTournamentRegistration& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Check in to a tournament */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournament")
	void CheckIn(
		const FString& TournamentId,
		const FOnTournamentRegistration& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Leave / unregister from a tournament */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournament")
	void Leave(
		const FString& TournamentId,
		const FOnSuccess& OnSuccess,
		const FOnDeskillzApiError& OnError);

	// ========================================================================
	// STATUS & SCHEDULE (3.6 - 3.9)
	// ========================================================================

	/** Get enrollment status for a tournament */
	void GetEnrollmentStatus(
		const FString& TournamentId,
		const FOnEnrollmentState& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Get all my tournament registrations */
	void GetMyRegistrations(
		const FOnTournamentRegistrations& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Get tournament bracket schedule */
	void GetSchedule(
		const FString& TournamentId,
		const FOnTournamentSchedule& OnSuccess,
		const FOnDeskillzApiError& OnError);

	/** Get my table assignment in current round */
	void GetMyTableAssignment(
		const FString& TournamentId,
		const FOnTableAssignment& OnSuccess,
		const FOnDeskillzApiError& OnError);

	// ========================================================================
	// DELEGATES (3.10)
	// ========================================================================

	/** Fired when registered for a tournament */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournament")
	FOnDeskillzTournamentsReceived OnTournamentsReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentStartedDelegate, const FString&, TournamentId);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournament")
	FOnTournamentStartedDelegate OnTournamentStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentLeftDelegate, const FString&, TournamentId);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Tournament")
	FOnTournamentLeftDelegate OnTournamentLeft;

	// ========================================================================
	// SOCKET EVENT HANDLERS (called by RealtimeEventRouter)
	// ========================================================================

	void HandleTournamentStarted(const FString& TournamentId);
	void HandleTournamentLeft(const FString& TournamentId);

private:
	/** Parse tournament listing from JSON */
	static FDeskillzTournamentListing ParseTournamentListing(const TSharedPtr<FJsonObject>& Json);

	/** Parse tournament registration from JSON */
	static FDeskillzTournamentRegistration ParseRegistration(const TSharedPtr<FJsonObject>& Json);

	/** Parse enrollment state from JSON */
	static FDeskillzEnrollmentState ParseEnrollmentState(const TSharedPtr<FJsonObject>& Json);

	/** Parse schedule from JSON */
	static FDeskillzTournamentSchedule ParseSchedule(const TSharedPtr<FJsonObject>& Json);

	/** Parse table assignment from JSON */
	static FDeskillzTableAssignment ParseTableAssignment(const TSharedPtr<FJsonObject>& Json);

	/** Generic error parser */
	static FDeskillzError ParseError(const FString& ResponseBody, int32 StatusCode);
};