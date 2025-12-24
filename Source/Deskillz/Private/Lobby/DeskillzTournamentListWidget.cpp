// Copyright Deskillz Games. All Rights Reserved.
// DeskillzTournamentListWidget_Updated.cpp - UPDATED for centralized lobby architecture

#include "Lobby/DeskillzTournamentListWidget_Updated.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "HAL/PlatformProcess.h"

UDeskillzTournamentListWidget_Updated::UDeskillzTournamentListWidget_Updated(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Constructor
}

void UDeskillzTournamentListWidget_Updated::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Set up text content
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(DisplayTitle));
	}
	
	if (DescriptionText)
	{
		DescriptionText->SetText(FText::FromString(DisplayDescription));
	}
	
	if (FeaturesText)
	{
		FeaturesText->SetText(FText::FromString(FeaturesListText));
	}
	
	if (ButtonText)
	{
		ButtonText->SetText(FText::FromString(TEXT("Open Deskillz")));
	}
	
	// Bind button clicks
	if (OpenAppButton)
	{
		OpenAppButton->OnClicked.AddDynamic(this, &UDeskillzTournamentListWidget_Updated::OnOpenAppClicked);
	}
	
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UDeskillzTournamentListWidget_Updated::OnCloseClicked);
	}
}

void UDeskillzTournamentListWidget_Updated::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	// Apply theme colors
	// This would set colors on text blocks and buttons based on Theme
}

void UDeskillzTournamentListWidget_Updated::OpenDeskillzApp()
{
	// Try app deep link first on mobile
#if PLATFORM_IOS || PLATFORM_ANDROID
	FString DeepLink = AppScheme + TEXT("tournaments");
	OpenURL(DeepLink);
#else
	// Desktop - open website
	OpenURL(WebsiteURL + TEXT("/tournaments"));
#endif
}

void UDeskillzTournamentListWidget_Updated::OpenTournament(const FString& TournamentId)
{
	if (TournamentId.IsEmpty())
	{
		OpenTournamentsList();
		return;
	}
	
#if PLATFORM_IOS || PLATFORM_ANDROID
	FString DeepLink = AppScheme + TEXT("tournament/") + TournamentId;
	OpenURL(DeepLink);
#else
	OpenURL(WebsiteURL + TEXT("/tournament/") + TournamentId);
#endif
}

void UDeskillzTournamentListWidget_Updated::OpenTournamentsList()
{
	OpenDeskillzApp();
}

void UDeskillzTournamentListWidget_Updated::OpenURL(const FString& URL)
{
	if (URL.IsEmpty())
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzTournamentListWidget] Opening URL: %s"), *URL);
	
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}

void UDeskillzTournamentListWidget_Updated::OnOpenAppClicked()
{
	OpenDeskillzApp();
}

void UDeskillzTournamentListWidget_Updated::OnCloseClicked()
{
	RemoveFromParent();
}