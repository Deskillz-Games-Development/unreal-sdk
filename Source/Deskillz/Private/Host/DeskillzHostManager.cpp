// =============================================================================
// Deskillz SDK for Unreal Engine - Host Manager Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "Host/DeskillzHostManager.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UDeskillzHostManager* UDeskillzHostManager::Instance = nullptr;

UDeskillzHostManager::UDeskillzHostManager()
    : bIsInitialized(false)
    , bIsRegisteredHost(false)
    , CurrentTier(0)
{
}

UDeskillzHostManager* UDeskillzHostManager::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzHostManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UDeskillzHostManager::Initialize(const FString& InHostId)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeskillzHostManager already initialized"));
        return;
    }

    HostId = InHostId;
    bIsInitialized = true;

    // Load cached host data if available
    LoadCachedHostData();

    UE_LOG(LogTemp, Log, TEXT("DeskillzHostManager initialized for host: %s"), *HostId);
}

void UDeskillzHostManager::Shutdown()
{
    if (!bIsInitialized) return;

    // Save current state
    SaveHostDataToCache();

    bIsInitialized = false;
    bIsRegisteredHost = false;
    ActiveRooms.Empty();

    UE_LOG(LogTemp, Log, TEXT("DeskillzHostManager shutdown"));
}

// =============================================================================
// Host Registration
// =============================================================================

void UDeskillzHostManager::RegisterAsHost(const FDeskillzHostRegistrationRequest& Request)
{
    if (!bIsInitialized)
    {
        OnHostRegistrationComplete.Broadcast(false, TEXT("HostManager not initialized"));
        return;
    }

    // Validate request
    if (Request.DisplayName.IsEmpty())
    {
        OnHostRegistrationComplete.Broadcast(false, TEXT("Display name is required"));
        return;
    }

    // Build API request
    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("displayName"), Request.DisplayName);
    JsonRequest->SetStringField(TEXT("email"), Request.Email);
    JsonRequest->SetBoolField(TEXT("acceptedTerms"), Request.bAcceptedTerms);

    // Send registration request to server
    SendHostApiRequest(TEXT("hosts/register"), TEXT("POST"), JsonRequest,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                bIsRegisteredHost = true;
                HostId = Response->GetStringField(TEXT("hostId"));
                CurrentTier = Response->GetIntegerField(TEXT("tier"));
                
                // Parse host profile
                ParseHostProfile(Response);
                
                SaveHostDataToCache();
                OnHostRegistrationComplete.Broadcast(true, TEXT(""));
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Registration failed");
                OnHostRegistrationComplete.Broadcast(false, ErrorMsg);
            }
        });
}

void UDeskillzHostManager::FetchHostProfile()
{
    if (!bIsInitialized || HostId.IsEmpty())
    {
        OnHostProfileUpdated.Broadcast(FDeskillzHostProfile());
        return;
    }

    FString Endpoint = FString::Printf(TEXT("hosts/%s/profile"), *HostId);
    
    SendHostApiRequest(Endpoint, TEXT("GET"), nullptr,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                ParseHostProfile(Response);
                OnHostProfileUpdated.Broadcast(CurrentProfile);
            }
            else
            {
                OnHostProfileUpdated.Broadcast(FDeskillzHostProfile());
            }
        });
}

void UDeskillzHostManager::UpdateHostProfile(const FDeskillzHostProfileUpdate& Update)
{
    if (!bIsRegisteredHost)
    {
        OnHostProfileUpdateComplete.Broadcast(false, TEXT("Not registered as host"));
        return;
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    
    if (!Update.DisplayName.IsEmpty())
        JsonRequest->SetStringField(TEXT("displayName"), Update.DisplayName);
    if (!Update.Bio.IsEmpty())
        JsonRequest->SetStringField(TEXT("bio"), Update.Bio);
    if (!Update.AvatarUrl.IsEmpty())
        JsonRequest->SetStringField(TEXT("avatarUrl"), Update.AvatarUrl);

    FString Endpoint = FString::Printf(TEXT("hosts/%s/profile"), *HostId);
    
    SendHostApiRequest(Endpoint, TEXT("PATCH"), JsonRequest,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess)
            {
                ParseHostProfile(Response);
                OnHostProfileUpdateComplete.Broadcast(true, TEXT(""));
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Update failed");
                OnHostProfileUpdateComplete.Broadcast(false, ErrorMsg);
            }
        });
}

// =============================================================================
// Tier System
// =============================================================================

int32 UDeskillzHostManager::GetCurrentTier() const
{
    return CurrentTier;
}

FDeskillzHostTierInfo UDeskillzHostManager::GetTierInfo(int32 Tier) const
{
    FDeskillzHostTierInfo Info;
    
    switch (Tier)
    {
        case 0:
            Info.TierLevel = 0;
            Info.TierName = TEXT("Starter");
            Info.RevenueSharePercent = 50.0f;
            Info.RequiredRoomsHosted = 0;
            Info.RequiredTotalEarnings = 0.0f;
            Info.RequiredRating = 0.0f;
            Info.MaxConcurrentRooms = 2;
            Info.bCanHostPremiumGames = false;
            Info.bHasPrioritySupport = false;
            break;
        case 1:
            Info.TierLevel = 1;
            Info.TierName = TEXT("Bronze");
            Info.RevenueSharePercent = 55.0f;
            Info.RequiredRoomsHosted = 10;
            Info.RequiredTotalEarnings = 100.0f;
            Info.RequiredRating = 3.5f;
            Info.MaxConcurrentRooms = 3;
            Info.bCanHostPremiumGames = false;
            Info.bHasPrioritySupport = false;
            break;
        case 2:
            Info.TierLevel = 2;
            Info.TierName = TEXT("Silver");
            Info.RevenueSharePercent = 60.0f;
            Info.RequiredRoomsHosted = 50;
            Info.RequiredTotalEarnings = 500.0f;
            Info.RequiredRating = 4.0f;
            Info.MaxConcurrentRooms = 5;
            Info.bCanHostPremiumGames = true;
            Info.bHasPrioritySupport = false;
            break;
        case 3:
            Info.TierLevel = 3;
            Info.TierName = TEXT("Gold");
            Info.RevenueSharePercent = 65.0f;
            Info.RequiredRoomsHosted = 150;
            Info.RequiredTotalEarnings = 2000.0f;
            Info.RequiredRating = 4.3f;
            Info.MaxConcurrentRooms = 8;
            Info.bCanHostPremiumGames = true;
            Info.bHasPrioritySupport = true;
            break;
        case 4:
            Info.TierLevel = 4;
            Info.TierName = TEXT("Platinum");
            Info.RevenueSharePercent = 70.0f;
            Info.RequiredRoomsHosted = 500;
            Info.RequiredTotalEarnings = 10000.0f;
            Info.RequiredRating = 4.5f;
            Info.MaxConcurrentRooms = 12;
            Info.bCanHostPremiumGames = true;
            Info.bHasPrioritySupport = true;
            break;
        case 5:
            Info.TierLevel = 5;
            Info.TierName = TEXT("Diamond");
            Info.RevenueSharePercent = 75.0f;
            Info.RequiredRoomsHosted = 1000;
            Info.RequiredTotalEarnings = 50000.0f;
            Info.RequiredRating = 4.7f;
            Info.MaxConcurrentRooms = 20;
            Info.bCanHostPremiumGames = true;
            Info.bHasPrioritySupport = true;
            break;
        default:
            Info.TierLevel = 0;
            Info.TierName = TEXT("Unknown");
            break;
    }
    
    return Info;
}

float UDeskillzHostManager::GetTierProgress() const
{
    if (CurrentTier >= 5) return 1.0f;

    FDeskillzHostTierInfo NextTier = GetTierInfo(CurrentTier + 1);
    
    float RoomsProgress = static_cast<float>(CurrentProfile.TotalRoomsHosted) / 
                          static_cast<float>(NextTier.RequiredRoomsHosted);
    float EarningsProgress = CurrentProfile.TotalEarnings / NextTier.RequiredTotalEarnings;
    float RatingProgress = CurrentProfile.AverageRating / NextTier.RequiredRating;

    // Average of all progress metrics
    return FMath::Clamp((RoomsProgress + EarningsProgress + RatingProgress) / 3.0f, 0.0f, 1.0f);
}

void UDeskillzHostManager::CheckTierUpgrade()
{
    if (CurrentTier >= 5) return;

    FDeskillzHostTierInfo NextTier = GetTierInfo(CurrentTier + 1);

    bool bMeetsRequirements = 
        CurrentProfile.TotalRoomsHosted >= NextTier.RequiredRoomsHosted &&
        CurrentProfile.TotalEarnings >= NextTier.RequiredTotalEarnings &&
        CurrentProfile.AverageRating >= NextTier.RequiredRating;

    if (bMeetsRequirements)
    {
        int32 OldTier = CurrentTier;
        CurrentTier = NextTier.TierLevel;
        
        // Notify server of tier upgrade
        NotifyTierUpgrade(OldTier, CurrentTier);
        
        OnTierUpgraded.Broadcast(OldTier, CurrentTier);
    }
}

// =============================================================================
// Room Management
// =============================================================================

void UDeskillzHostManager::CreateRoom(const FDeskillzCreateRoomRequest& Request)
{
    if (!bIsRegisteredHost)
    {
        OnRoomCreated.Broadcast(false, TEXT(""), TEXT("Not registered as host"));
        return;
    }

    FDeskillzHostTierInfo TierInfo = GetTierInfo(CurrentTier);
    if (ActiveRooms.Num() >= TierInfo.MaxConcurrentRooms)
    {
        OnRoomCreated.Broadcast(false, TEXT(""), TEXT("Maximum concurrent rooms reached for your tier"));
        return;
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("name"), Request.RoomName);
    JsonRequest->SetStringField(TEXT("gameId"), Request.GameId);
    JsonRequest->SetNumberField(TEXT("maxPlayers"), Request.MaxPlayers);
    JsonRequest->SetNumberField(TEXT("minBuyIn"), Request.MinBuyIn);
    JsonRequest->SetNumberField(TEXT("maxBuyIn"), Request.MaxBuyIn);
    JsonRequest->SetNumberField(TEXT("pointValue"), Request.PointValue);
    JsonRequest->SetNumberField(TEXT("rakePercent"), Request.RakePercent);
    JsonRequest->SetNumberField(TEXT("rakeCap"), Request.RakeCap);
    JsonRequest->SetBoolField(TEXT("isPrivate"), Request.bIsPrivate);
    
    if (!Request.Password.IsEmpty())
        JsonRequest->SetStringField(TEXT("password"), Request.Password);

    SendHostApiRequest(TEXT("rooms/create"), TEXT("POST"), JsonRequest,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                FString RoomId = Response->GetStringField(TEXT("roomId"));
                
                FDeskillzActiveRoom NewRoom;
                NewRoom.RoomId = RoomId;
                NewRoom.RoomName = Response->GetStringField(TEXT("name"));
                NewRoom.GameId = Response->GetStringField(TEXT("gameId"));
                NewRoom.CurrentPlayers = 0;
                NewRoom.MaxPlayers = Response->GetIntegerField(TEXT("maxPlayers"));
                NewRoom.Status = EDeskillzRoomStatus::WaitingForPlayers;
                NewRoom.CreatedAt = FDateTime::UtcNow();
                
                ActiveRooms.Add(RoomId, NewRoom);
                
                OnRoomCreated.Broadcast(true, RoomId, TEXT(""));
                OnActiveRoomsChanged.Broadcast(GetActiveRooms());
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Failed to create room");
                OnRoomCreated.Broadcast(false, TEXT(""), ErrorMsg);
            }
        });
}

void UDeskillzHostManager::CloseRoom(const FString& RoomId, const FString& Reason)
{
    if (!ActiveRooms.Contains(RoomId))
    {
        OnRoomClosed.Broadcast(false, RoomId, TEXT("Room not found"));
        return;
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("reason"), Reason);

    FString Endpoint = FString::Printf(TEXT("rooms/%s/close"), *RoomId);
    
    SendHostApiRequest(Endpoint, TEXT("POST"), JsonRequest,
        [this, RoomId](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess)
            {
                ActiveRooms.Remove(RoomId);
                OnRoomClosed.Broadcast(true, RoomId, TEXT(""));
                OnActiveRoomsChanged.Broadcast(GetActiveRooms());
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Failed to close room");
                OnRoomClosed.Broadcast(false, RoomId, ErrorMsg);
            }
        });
}

void UDeskillzHostManager::UpdateRoomSettings(const FString& RoomId, const FDeskillzRoomSettingsUpdate& Settings)
{
    if (!ActiveRooms.Contains(RoomId))
    {
        OnRoomSettingsUpdated.Broadcast(false, RoomId, TEXT("Room not found"));
        return;
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    
    if (Settings.MaxPlayers > 0)
        JsonRequest->SetNumberField(TEXT("maxPlayers"), Settings.MaxPlayers);
    if (Settings.PointValue > 0)
        JsonRequest->SetNumberField(TEXT("pointValue"), Settings.PointValue);
    if (Settings.RakePercent >= 0)
        JsonRequest->SetNumberField(TEXT("rakePercent"), Settings.RakePercent);

    FString Endpoint = FString::Printf(TEXT("rooms/%s/settings"), *RoomId);
    
    SendHostApiRequest(Endpoint, TEXT("PATCH"), JsonRequest,
        [this, RoomId](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess)
            {
                OnRoomSettingsUpdated.Broadcast(true, RoomId, TEXT(""));
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Failed to update settings");
                OnRoomSettingsUpdated.Broadcast(false, RoomId, ErrorMsg);
            }
        });
}

TArray<FDeskillzActiveRoom> UDeskillzHostManager::GetActiveRooms() const
{
    TArray<FDeskillzActiveRoom> Result;
    ActiveRooms.GenerateValueArray(Result);
    return Result;
}

FDeskillzActiveRoom UDeskillzHostManager::GetRoomInfo(const FString& RoomId) const
{
    if (const FDeskillzActiveRoom* Room = ActiveRooms.Find(RoomId))
    {
        return *Room;
    }
    return FDeskillzActiveRoom();
}

// =============================================================================
// Earnings & Payouts
// =============================================================================

void UDeskillzHostManager::FetchEarnings(EDeskillzEarningsPeriod Period)
{
    if (!bIsRegisteredHost)
    {
        OnEarningsUpdated.Broadcast(FDeskillzHostEarnings());
        return;
    }

    FString PeriodStr;
    switch (Period)
    {
        case EDeskillzEarningsPeriod::Today: PeriodStr = TEXT("today"); break;
        case EDeskillzEarningsPeriod::Week: PeriodStr = TEXT("week"); break;
        case EDeskillzEarningsPeriod::Month: PeriodStr = TEXT("month"); break;
        case EDeskillzEarningsPeriod::Year: PeriodStr = TEXT("year"); break;
        case EDeskillzEarningsPeriod::AllTime: PeriodStr = TEXT("all"); break;
    }

    FString Endpoint = FString::Printf(TEXT("hosts/%s/earnings?period=%s"), *HostId, *PeriodStr);
    
    SendHostApiRequest(Endpoint, TEXT("GET"), nullptr,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                FDeskillzHostEarnings Earnings;
                Earnings.TotalEarnings = Response->GetNumberField(TEXT("totalEarnings"));
                Earnings.RakeEarnings = Response->GetNumberField(TEXT("rakeEarnings"));
                Earnings.BonusEarnings = Response->GetNumberField(TEXT("bonusEarnings"));
                Earnings.ReferralEarnings = Response->GetNumberField(TEXT("referralEarnings"));
                Earnings.PendingPayout = Response->GetNumberField(TEXT("pendingPayout"));
                Earnings.AvailableBalance = Response->GetNumberField(TEXT("availableBalance"));
                
                OnEarningsUpdated.Broadcast(Earnings);
            }
            else
            {
                OnEarningsUpdated.Broadcast(FDeskillzHostEarnings());
            }
        });
}

void UDeskillzHostManager::RequestPayout(float Amount, const FString& WalletAddress)
{
    if (!bIsRegisteredHost)
    {
        OnPayoutRequested.Broadcast(false, TEXT("Not registered as host"));
        return;
    }

    if (Amount <= 0)
    {
        OnPayoutRequested.Broadcast(false, TEXT("Invalid payout amount"));
        return;
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetNumberField(TEXT("amount"), Amount);
    JsonRequest->SetStringField(TEXT("walletAddress"), WalletAddress);

    FString Endpoint = FString::Printf(TEXT("hosts/%s/payout"), *HostId);
    
    SendHostApiRequest(Endpoint, TEXT("POST"), JsonRequest,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess)
            {
                OnPayoutRequested.Broadcast(true, TEXT(""));
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Payout request failed");
                OnPayoutRequested.Broadcast(false, ErrorMsg);
            }
        });
}

// =============================================================================
// Private Methods
// =============================================================================

void UDeskillzHostManager::ParseHostProfile(TSharedPtr<FJsonObject> JsonData)
{
    if (!JsonData.IsValid()) return;

    CurrentProfile.HostId = JsonData->GetStringField(TEXT("hostId"));
    CurrentProfile.DisplayName = JsonData->GetStringField(TEXT("displayName"));
    CurrentProfile.Email = JsonData->GetStringField(TEXT("email"));
    CurrentProfile.AvatarUrl = JsonData->GetStringField(TEXT("avatarUrl"));
    CurrentProfile.Bio = JsonData->GetStringField(TEXT("bio"));
    CurrentProfile.TierLevel = JsonData->GetIntegerField(TEXT("tier"));
    CurrentProfile.TotalRoomsHosted = JsonData->GetIntegerField(TEXT("totalRoomsHosted"));
    CurrentProfile.TotalPlayersServed = JsonData->GetIntegerField(TEXT("totalPlayersServed"));
    CurrentProfile.TotalEarnings = JsonData->GetNumberField(TEXT("totalEarnings"));
    CurrentProfile.AverageRating = JsonData->GetNumberField(TEXT("averageRating"));
    CurrentProfile.bIsVerified = JsonData->GetBoolField(TEXT("isVerified"));
    
    CurrentTier = CurrentProfile.TierLevel;
}

void UDeskillzHostManager::SendHostApiRequest(const FString& Endpoint, const FString& Method, 
    TSharedPtr<FJsonObject> Body, TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback)
{
    // Implementation would use HTTP module to send requests
    // This is a placeholder showing the structure
    
    FString Url = FString::Printf(TEXT("%s/api/v1/%s"), *ApiBaseUrl, *Endpoint);
    
    // Create HTTP request
    // Set headers including auth token
    // Send request
    // Parse response and call callback
    
    UE_LOG(LogTemp, Log, TEXT("Host API Request: %s %s"), *Method, *Url);
}

void UDeskillzHostManager::NotifyTierUpgrade(int32 OldTier, int32 NewTier)
{
    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetNumberField(TEXT("oldTier"), OldTier);
    JsonRequest->SetNumberField(TEXT("newTier"), NewTier);

    SendHostApiRequest(TEXT("hosts/tier-upgrade"), TEXT("POST"), JsonRequest, nullptr);
}

void UDeskillzHostManager::LoadCachedHostData()
{
    // Load from local storage/file
    // Implementation would use SaveGame or config files
}

void UDeskillzHostManager::SaveHostDataToCache()
{
    // Save to local storage/file
    // Implementation would use SaveGame or config files
}