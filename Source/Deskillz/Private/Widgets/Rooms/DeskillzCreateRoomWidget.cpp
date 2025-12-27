// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzCreateRoomWidget.h"
#include "Rooms/DeskillzRooms.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Throbber.h"
#include "Components/CanvasPanel.h"

UDeskillzCreateRoomWidget::UDeskillzCreateRoomWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DefaultCurrency(TEXT("USDT"))
	, MinPlayersLimit(2)
	, MaxPlayersLimit(10)
	, bIsCreating(false)
	, bIsVisible(false)
{
	// Default currencies
	AvailableCurrencies = { TEXT("USDT"), TEXT("USDC"), TEXT("BTC"), TEXT("ETH"), TEXT("BNB"), TEXT("SOL"), TEXT("XRP") };
}

void UDeskillzCreateRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeUI();
}

void UDeskillzCreateRoomWidget::InitializeUI()
{
	// Bind buttons
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzCreateRoomWidget::OnBackButtonClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UDeskillzCreateRoomWidget::OnCancelButtonClicked);
	}
	
	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &UDeskillzCreateRoomWidget::OnCreateButtonClicked);
	}
	
	// Bind sliders
	if (MinPlayersSlider)
	{
		MinPlayersSlider->SetMinValue(2);
		MinPlayersSlider->SetMaxValue(MaxPlayersLimit);
		MinPlayersSlider->SetValue(2);
		MinPlayersSlider->OnValueChanged.AddDynamic(this, &UDeskillzCreateRoomWidget::OnMinPlayersSliderChanged);
	}
	
	if (MaxPlayersSlider)
	{
		MaxPlayersSlider->SetMinValue(2);
		MaxPlayersSlider->SetMaxValue(MaxPlayersLimit);
		MaxPlayersSlider->SetValue(2);
		MaxPlayersSlider->OnValueChanged.AddDynamic(this, &UDeskillzCreateRoomWidget::OnMaxPlayersSliderChanged);
	}
	
	// Bind name input
	if (NameInput)
	{
		NameInput->OnTextChanged.AddDynamic(this, &UDeskillzCreateRoomWidget::OnNameInputChanged);
	}
	
	// Populate dropdowns
	PopulateDropdowns();
	
	// Hide loading and error
	SetLoading(false);
	ClearError();
}

void UDeskillzCreateRoomWidget::PopulateDropdowns()
{
	// Currency dropdown
	if (CurrencyDropdown)
	{
		CurrencyDropdown->ClearOptions();
		for (const FString& Currency : AvailableCurrencies)
		{
			CurrencyDropdown->AddOption(Currency);
		}
		
		int32 DefaultIndex = AvailableCurrencies.Find(DefaultCurrency);
		CurrencyDropdown->SetSelectedIndex(DefaultIndex >= 0 ? DefaultIndex : 0);
	}
	
	// Visibility dropdown
	if (VisibilityDropdown)
	{
		VisibilityDropdown->ClearOptions();
		VisibilityDropdown->AddOption(TEXT("Unlisted (Code Only)"));
		VisibilityDropdown->AddOption(TEXT("Public"));
		VisibilityDropdown->AddOption(TEXT("Private (Invite Only)"));
		VisibilityDropdown->SetSelectedIndex(0);
	}
	
	// Mode dropdown
	if (ModeDropdown)
	{
		ModeDropdown->ClearOptions();
		ModeDropdown->AddOption(TEXT("Synchronous (Real-time)"));
		ModeDropdown->AddOption(TEXT("Asynchronous (Turn-based)"));
		ModeDropdown->SetSelectedIndex(0);
	}
}

// =============================================================================
// Public Methods
// =============================================================================

void UDeskillzCreateRoomWidget::Show()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
	ResetForm();
}

void UDeskillzCreateRoomWidget::Hide()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDeskillzCreateRoomWidget::ResetForm()
{
	if (NameInput)
	{
		NameInput->SetText(FText::GetEmpty());
	}
	
	if (DescriptionInput)
	{
		DescriptionInput->SetText(FText::GetEmpty());
	}
	
	if (EntryFeeInput)
	{
		EntryFeeInput->SetText(FText::FromString(TEXT("1.00")));
	}
	
	if (CurrencyDropdown)
	{
		int32 DefaultIndex = AvailableCurrencies.Find(DefaultCurrency);
		CurrencyDropdown->SetSelectedIndex(DefaultIndex >= 0 ? DefaultIndex : 0);
	}
	
	if (MinPlayersSlider)
	{
		MinPlayersSlider->SetValue(2);
	}
	
	if (MaxPlayersSlider)
	{
		MaxPlayersSlider->SetValue(2);
	}
	
	if (MinPlayersText)
	{
		MinPlayersText->SetText(FText::FromString(TEXT("Min Players: 2")));
	}
	
	if (MaxPlayersText)
	{
		MaxPlayersText->SetText(FText::FromString(TEXT("Max Players: 2")));
	}
	
	if (VisibilityDropdown)
	{
		VisibilityDropdown->SetSelectedIndex(0);
	}
	
	if (ModeDropdown)
	{
		ModeDropdown->SetSelectedIndex(0);
	}
	
	if (InviteRequiredCheckbox)
	{
		InviteRequiredCheckbox->SetIsChecked(false);
	}
	
	ClearError();
}

// =============================================================================
// Validation
// =============================================================================

bool UDeskillzCreateRoomWidget::ValidateForm()
{
	// Room name required
	FString RoomName = NameInput ? NameInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
	if (RoomName.IsEmpty())
	{
		ShowError(TEXT("Room name is required"));
		return false;
	}
	
	if (RoomName.Len() < 3)
	{
		ShowError(TEXT("Room name must be at least 3 characters"));
		return false;
	}
	
	// Entry fee validation
	FString EntryFeeStr = EntryFeeInput ? EntryFeeInput->GetText().ToString() : TEXT("0");
	float EntryFee = FCString::Atof(*EntryFeeStr);
	if (EntryFee < 0)
	{
		ShowError(TEXT("Invalid entry fee"));
		return false;
	}
	
	// Player count validation
	int32 MinPlayers = MinPlayersSlider ? FMath::RoundToInt(MinPlayersSlider->GetValue()) : 2;
	int32 MaxPlayers = MaxPlayersSlider ? FMath::RoundToInt(MaxPlayersSlider->GetValue()) : 2;
	if (MinPlayers > MaxPlayers)
	{
		ShowError(TEXT("Min players cannot exceed max players"));
		return false;
	}
	
	return true;
}

ERoomVisibility UDeskillzCreateRoomWidget::GetSelectedVisibility() const
{
	int32 Index = VisibilityDropdown ? VisibilityDropdown->GetSelectedIndex() : 0;
	switch (Index)
	{
	case 0: return ERoomVisibility::Unlisted;
	case 1: return ERoomVisibility::PublicListed;
	case 2: return ERoomVisibility::Private;
	default: return ERoomVisibility::Unlisted;
	}
}

ERoomMode UDeskillzCreateRoomWidget::GetSelectedMode() const
{
	int32 Index = ModeDropdown ? ModeDropdown->GetSelectedIndex() : 0;
	return Index == 0 ? ERoomMode::Sync : ERoomMode::Async;
}

FCreateRoomConfig UDeskillzCreateRoomWidget::BuildRoomConfig() const
{
	FCreateRoomConfig Config;
	
	Config.Name = NameInput ? NameInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
	Config.Description = DescriptionInput ? DescriptionInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
	
	FString EntryFeeStr = EntryFeeInput ? EntryFeeInput->GetText().ToString() : TEXT("0");
	Config.EntryFee = FCString::Atof(*EntryFeeStr);
	
	Config.EntryCurrency = CurrencyDropdown ? CurrencyDropdown->GetSelectedOption() : DefaultCurrency;
	Config.MinPlayers = MinPlayersSlider ? FMath::RoundToInt(MinPlayersSlider->GetValue()) : 2;
	Config.MaxPlayers = MaxPlayersSlider ? FMath::RoundToInt(MaxPlayersSlider->GetValue()) : 2;
	Config.Visibility = GetSelectedVisibility();
	Config.Mode = GetSelectedMode();
	Config.bInviteRequired = InviteRequiredCheckbox ? InviteRequiredCheckbox->IsChecked() : false;
	
	return Config;
}

void UDeskillzCreateRoomWidget::ShowError(const FString& Message)
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(Message));
		ErrorText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDeskillzCreateRoomWidget::ClearError()
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeskillzCreateRoomWidget::SetLoading(bool bLoading)
{
	bIsCreating = bLoading;
	
	if (LoadingOverlay)
	{
		LoadingOverlay->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(!bLoading);
	}
	
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(!bLoading);
	}
	
	if (BackButton)
	{
		BackButton->SetIsEnabled(!bLoading);
	}
}

// =============================================================================
// UI Callbacks
// =============================================================================

void UDeskillzCreateRoomWidget::OnBackButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UDeskillzCreateRoomWidget::OnCancelButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UDeskillzCreateRoomWidget::OnCreateButtonClicked()
{
	if (bIsCreating)
	{
		return;
	}
	
	if (!ValidateForm())
	{
		return;
	}
	
	FCreateRoomConfig Config = BuildRoomConfig();
	
	SetLoading(true);
	
	UDeskillzRooms* RoomSystem = UDeskillzRooms::Get(this);
	if (!RoomSystem)
	{
		ShowError(TEXT("Room service not available"));
		SetLoading(false);
		return;
	}
	
	FOnRoomSuccess OnSuccess;
	OnSuccess.BindLambda([this](const FPrivateRoom& Room)
	{
		SetLoading(false);
		OnRoomCreated.Broadcast(Room);
		UE_LOG(LogTemp, Log, TEXT("[CreateRoomWidget] Room created: %s"), *Room.RoomCode);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		SetLoading(false);
		ShowError(Error.Message);
	});
	
	RoomSystem->CreateRoom(Config, OnSuccess, OnError);
}

void UDeskillzCreateRoomWidget::OnMinPlayersSliderChanged(float Value)
{
	int32 IntValue = FMath::RoundToInt(Value);
	
	if (MinPlayersText)
	{
		MinPlayersText->SetText(FText::FromString(FString::Printf(TEXT("Min Players: %d"), IntValue)));
	}
	
	// Ensure max >= min
	if (MaxPlayersSlider && MaxPlayersSlider->GetValue() < Value)
	{
		MaxPlayersSlider->SetValue(Value);
	}
}

void UDeskillzCreateRoomWidget::OnMaxPlayersSliderChanged(float Value)
{
	int32 IntValue = FMath::RoundToInt(Value);
	
	if (MaxPlayersText)
	{
		MaxPlayersText->SetText(FText::FromString(FString::Printf(TEXT("Max Players: %d"), IntValue)));
	}
	
	// Ensure min <= max
	if (MinPlayersSlider && MinPlayersSlider->GetValue() > Value)
	{
		MinPlayersSlider->SetValue(Value);
	}
}

void UDeskillzCreateRoomWidget::OnNameInputChanged(const FText& Text)
{
	ClearError();
}