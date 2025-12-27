// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzRoomPlayerCard.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"

UDeskillzRoomPlayerCard::UDeskillzRoomPlayerCard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ReadyColor(FLinearColor(0.3f, 0.9f, 0.4f, 1.0f))
	, NotReadyColor(FLinearColor(0.4f, 0.4f, 0.45f, 1.0f))
	, CurrentUserBackgroundColor(FLinearColor(0.18f, 0.18f, 0.22f, 1.0f))
	, DefaultBackgroundColor(FLinearColor(0.15f, 0.15f, 0.18f, 1.0f))
	, bCanKickPlayer(false)
{
}

void UDeskillzRoomPlayerCard::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeUI();
}

void UDeskillzRoomPlayerCard::InitializeUI()
{
	if (KickButton)
	{
		KickButton->OnClicked.AddDynamic(this, &UDeskillzRoomPlayerCard::OnKickButtonClicked);
	}
}

// =============================================================================
// Public Methods
// =============================================================================

void UDeskillzRoomPlayerCard::SetPlayer(const FRoomPlayer& Player, bool bCanKick)
{
	CurrentPlayer = Player;
	bCanKickPlayer = bCanKick;
	UpdateDisplay();
}

void UDeskillzRoomPlayerCard::SetReady(bool bReady)
{
	CurrentPlayer.bIsReady = bReady;
	UpdateReadyStatus();
}

// =============================================================================
// Display Updates
// =============================================================================

void UDeskillzRoomPlayerCard::UpdateDisplay()
{
	// Username
	if (UsernameText)
	{
		FString DisplayName = CurrentPlayer.Username;
		if (CurrentPlayer.bIsCurrentUser)
		{
			DisplayName += TEXT(" (You)");
		}
		UsernameText->SetText(FText::FromString(DisplayName));
	}
	
	// Host badge
	if (HostBadge)
	{
		HostBadge->SetVisibility(CurrentPlayer.bIsAdmin ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	// Background color for current user
	if (CardBackground)
	{
		FLinearColor BgColor = CurrentPlayer.bIsCurrentUser ? CurrentUserBackgroundColor : DefaultBackgroundColor;
		CardBackground->SetBrushColor(BgColor);
	}
	
	// Kick button visibility
	if (KickButton)
	{
		KickButton->SetVisibility(bCanKickPlayer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	// Ready status
	UpdateReadyStatus();
	
	// Avatar
	LoadAvatar();
}

void UDeskillzRoomPlayerCard::UpdateReadyStatus()
{
	if (StatusText)
	{
		if (CurrentPlayer.bIsReady)
		{
			StatusText->SetText(FText::FromString(TEXT("Ready")));
			StatusText->SetColorAndOpacity(FSlateColor(ReadyColor));
		}
		else
		{
			StatusText->SetText(FText::FromString(TEXT("Not Ready")));
			StatusText->SetColorAndOpacity(FSlateColor(NotReadyColor));
		}
	}
	
	if (ReadyIndicator)
	{
		FLinearColor IndicatorColor = CurrentPlayer.bIsReady ? ReadyColor : NotReadyColor;
		ReadyIndicator->SetBrushColor(IndicatorColor);
	}
}

void UDeskillzRoomPlayerCard::LoadAvatar()
{
	if (!AvatarImage)
	{
		return;
	}
	
	// For now, use a solid color based on username hash
	// In production, load from URL using AsyncTaskDownloadImage
	FLinearColor AvatarColor = GetAvatarColor(CurrentPlayer.Username);
	AvatarImage->SetColorAndOpacity(AvatarColor);
	
	// TODO: Load actual avatar from URL
	// if (!CurrentPlayer.AvatarUrl.IsEmpty())
	// {
	//     UAsyncTaskDownloadImage::DownloadImage(CurrentPlayer.AvatarUrl, 
	//         FOnDownloadComplete::CreateUObject(this, &UDeskillzRoomPlayerCard::OnAvatarLoaded));
	// }
}

FLinearColor UDeskillzRoomPlayerCard::GetAvatarColor(const FString& Username) const
{
	if (Username.IsEmpty())
	{
		return FLinearColor(0.3f, 0.3f, 0.35f, 1.0f);
	}
	
	// Generate hue from username hash
	uint32 Hash = GetTypeHash(Username);
	float Hue = FMath::Abs((int32)(Hash % 360)) / 360.0f;
	
	// Convert HSV to RGB (simplified)
	FLinearColor Color = FLinearColor::MakeFromHSV8(
		(uint8)(Hue * 255), 
		128,  // 50% saturation
		153   // 60% value
	);
	
	return Color;
}

// =============================================================================
// UI Callbacks
// =============================================================================

void UDeskillzRoomPlayerCard::OnKickButtonClicked()
{
	if (!CurrentPlayer.Id.IsEmpty())
	{
		OnKickClicked.Broadcast(CurrentPlayer.Id);
	}
}