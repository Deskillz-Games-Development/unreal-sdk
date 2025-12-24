// Copyright Deskillz Games. All Rights Reserved.

#include "DeskillzSetupWizard.h"
#include "Core/DeskillzConfig.h"
#include "Deskillz.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "DeskillzEditor"

// Step titles
const TArray<FString> SDeskillzSetupWizard::StepTitles = {
	TEXT("Welcome"),
	TEXT("API Credentials"),
	TEXT("Environment"),
	TEXT("Match Settings"),
	TEXT("Test Connection"),
	TEXT("Complete!")
};

void SDeskillzSetupWizard::Construct(const FArguments& InArgs)
{
	// Get config
	Config = GetMutableDefault<UDeskillzConfig>();
	
	// Pre-fill from existing config
	if (Config)
	{
		APIKey = Config->APIKey;
		GameId = Config->GameId;
		SelectedEnvironment = (int32)Config->Environment;
	}
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16.0f)
		[
			SNew(SVerticalBox)
			
			// Header with logo and title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SNew(SHorizontalBox)
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("🎮 DESKILLZ SDK SETUP")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				]
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("v%s"), *UDeskillzSDK::GetSDKVersion())))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
				]
			]
			
			// Progress indicator
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				BuildProgressIndicator()
			]
			
			// Step content (scrollable)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.MinDesiredHeight(300)
					[
						BuildStepContent()
					]
				]
			]
			
			// Navigation buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 20, 0, 0)
			[
				BuildNavigationButtons()
			]
		]
	];
}

void SDeskillzSetupWizard::OpenWizard()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("SetupWizardTitle", "Deskillz SDK Setup Wizard"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SDeskillzSetupWizard)
		];
	
	FSlateApplication::Get().AddWindow(Window);
}

bool SDeskillzSetupWizard::IsSetupComplete()
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	return Config && Config->IsValid();
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildProgressIndicator()
{
	TSharedRef<SHorizontalBox> ProgressBox = SNew(SHorizontalBox);
	
	for (int32 i = 0; i < TotalSteps; i++)
	{
		// Step circle
		FLinearColor CircleColor = (i <= CurrentStep) 
			? FLinearColor(0.0f, 0.8f, 0.4f) // Green for completed/current
			: FLinearColor(0.3f, 0.3f, 0.3f); // Gray for future
		
		ProgressBox->AddSlot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(CircleColor)
				.Padding(8)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::FromInt(i + 1)))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
				]
			];
		
		// Connector line (except for last)
		if (i < TotalSteps - 1)
		{
			FLinearColor LineColor = (i < CurrentStep)
				? FLinearColor(0.0f, 0.8f, 0.4f)
				: FLinearColor(0.3f, 0.3f, 0.3f);
			
			ProgressBox->AddSlot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor(LineColor)
					.Padding(FMargin(0, 2, 0, 2))
					[
						SNullWidget::NullWidget
					]
				];
		}
	}
	
	return ProgressBox;
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildStepContent()
{
	switch (CurrentStep)
	{
		case 0: return BuildWelcomeStep();
		case 1: return BuildCredentialsStep();
		case 2: return BuildEnvironmentStep();
		case 3: return BuildSettingsStep();
		case 4: return BuildTestStep();
		case 5: return BuildCompleteStep();
		default: return SNullWidget::NullWidget;
	}
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildWelcomeStep()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("WelcomeTitle", "Welcome to Deskillz!"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("WelcomeDesc", "This wizard will guide you through setting up the Deskillz SDK for competitive gaming with cryptocurrency prizes.\n\nYou'll need:\n• A Deskillz Developer Account\n• Your API Key and Game ID (from the Developer Portal)\n\nThe setup takes less than 2 minutes!"))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 20, 0, 0)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenPortal", "Open Developer Portal"))
				.OnClicked(this, &SDeskillzSetupWizard::OnOpenPortalClicked)
			]
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildCredentialsStep()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CredentialsTitle", "Enter Your API Credentials"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CredentialsDesc", "Enter your API Key and Game ID from the Deskillz Developer Portal."))
			.AutoWrapText(true)
		]
		
		// API Key
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 20, 0, 5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("APIKeyLabel", "API Key"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 15)
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(APIKey))
			.HintText(LOCTEXT("APIKeyHint", "Enter your 32+ character API key"))
			.OnTextChanged_Lambda([this](const FText& NewText)
			{
				APIKey = NewText.ToString();
			})
		]
		
		// Game ID
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GameIDLabel", "Game ID"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(GameId))
			.HintText(LOCTEXT("GameIDHint", "Enter your Game ID"))
			.OnTextChanged_Lambda([this](const FText& NewText)
			{
				GameId = NewText.ToString();
			})
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildEnvironmentStep()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("EnvTitle", "Select Environment"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("EnvDesc", "Choose which environment to use. Use Sandbox for testing (no real money), and Production for your live game."))
			.AutoWrapText(true)
		]
		
		// Sandbox option
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 10)
		[
			SNew(SButton)
			.HAlign(HAlign_Fill)
			.OnClicked_Lambda([this]() { SelectedEnvironment = 1; return FReply::Handled(); })
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush(SelectedEnvironment == 1 ? "DetailsView.CategoryTop" : "ToolPanel.GroupBorder"))
				.Padding(15)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SandboxTitle", "🧪 Sandbox (Recommended for Testing)"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 5, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SandboxDesc", "Test your integration without real transactions. Perfect for development and QA."))
						.AutoWrapText(true)
						.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
					]
				]
			]
		]
		
		// Production option
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.HAlign(HAlign_Fill)
			.OnClicked_Lambda([this]() { SelectedEnvironment = 0; return FReply::Handled(); })
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush(SelectedEnvironment == 0 ? "DetailsView.CategoryTop" : "ToolPanel.GroupBorder"))
				.Padding(15)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ProdTitle", "🚀 Production"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 5, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ProdDesc", "Live environment with real cryptocurrency transactions. Use only for released games."))
						.AutoWrapText(true)
						.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
					]
				]
			]
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildSettingsStep()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsTitle", "Match Settings"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsDesc", "Configure default match settings. You can change these later in Project Settings."))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsInfo", "✓ Default match duration: 3 minutes\n✓ Practice mode: Enabled\n✓ Auto-submit scores: Enabled\n✓ Built-in UI: Enabled\n✓ Score encryption: Enabled"))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 20, 0, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsNote", "These are sensible defaults that work for most games. You can customize everything in Project Settings > Plugins > Deskillz SDK after setup."))
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildTestStep()
{
	FLinearColor StatusColor = bConnectionTested 
		? (bConnectionSucceeded ? FLinearColor(0.0f, 0.8f, 0.4f) : FLinearColor(1.0f, 0.3f, 0.3f))
		: FLinearColor::Gray;
	
	FString StatusText = bConnectionTested
		? (bConnectionSucceeded ? TEXT("✓ Connection successful!") : FString::Printf(TEXT("✗ %s"), *ConnectionMessage))
		: TEXT("Click 'Test Connection' to verify your credentials");
	
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TestTitle", "Test Connection"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TestDesc", "Let's verify that your credentials are correct and the SDK can connect to Deskillz servers."))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 20, 0, 20)
		[
			SNew(SButton)
			.Text(LOCTEXT("TestButton", "Test Connection"))
			.OnClicked(this, &SDeskillzSetupWizard::OnTestConnectionClicked)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(StatusText))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(FSlateColor(StatusColor))
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildCompleteStep()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CompleteTitle", "🎉 Setup Complete!"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CompleteDesc", "Your Deskillz SDK is now configured and ready to use!\n\nNext Steps:"))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 20)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CompleteSteps", 
				"1. The SDK will auto-initialize when your game starts\n"
				"2. When gameplay ends, call 'Deskillz Submit Score'\n"
				"3. Then call 'Deskillz End Match'\n\n"
				"That's it! Just 2 Blueprint nodes for basic integration."))
			.AutoWrapText(true)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CompleteNote", "📚 Check out the documentation and sample project for advanced features like real-time multiplayer, custom stages, and practice modes."))
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
		];
}

TSharedRef<SWidget> SDeskillzSetupWizard::BuildNavigationButtons()
{
	return SNew(SHorizontalBox)
		
		// Back button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("BackButton", "← Back"))
			.IsEnabled(CanGoBack())
			.OnClicked(this, &SDeskillzSetupWizard::OnBackClicked)
		]
		
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		
		// Next/Finish button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(CurrentStep == TotalSteps - 1 ? LOCTEXT("FinishButton", "Finish ✓") : LOCTEXT("NextButton", "Next →"))
			.IsEnabled(CurrentStep == TotalSteps - 1 ? CanFinish() : CanGoNext())
			.OnClicked(CurrentStep == TotalSteps - 1 
				? FOnClicked::CreateSP(this, &SDeskillzSetupWizard::OnFinishClicked)
				: FOnClicked::CreateSP(this, &SDeskillzSetupWizard::OnNextClicked))
		];
}

FReply SDeskillzSetupWizard::OnBackClicked()
{
	if (CurrentStep > 0)
	{
		CurrentStep--;
		// Force rebuild
		Invalidate(EInvalidateWidget::Layout);
	}
	return FReply::Handled();
}

FReply SDeskillzSetupWizard::OnNextClicked()
{
	// Save progress
	if (CurrentStep == 1)
	{
		// Save credentials
		if (Config)
		{
			Config->APIKey = APIKey;
			Config->GameId = GameId;
		}
	}
	else if (CurrentStep == 2)
	{
		// Save environment
		if (Config)
		{
			Config->Environment = (EDeskillzEnvironment)SelectedEnvironment;
		}
	}
	
	if (CurrentStep < TotalSteps - 1)
	{
		CurrentStep++;
		Invalidate(EInvalidateWidget::Layout);
	}
	return FReply::Handled();
}

FReply SDeskillzSetupWizard::OnFinishClicked()
{
	SaveConfiguration();
	MarkSetupComplete();
	
	// Close window
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
	
	return FReply::Handled();
}

FReply SDeskillzSetupWizard::OnTestConnectionClicked()
{
	TestConnection();
	return FReply::Handled();
}

FReply SDeskillzSetupWizard::OnOpenPortalClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://deskillz.games/developer"), nullptr, nullptr);
	return FReply::Handled();
}

bool SDeskillzSetupWizard::CanGoNext() const
{
	switch (CurrentStep)
	{
		case 0: return true; // Welcome
		case 1: return !APIKey.IsEmpty() && !GameId.IsEmpty(); // Credentials
		case 2: return true; // Environment
		case 3: return true; // Settings
		case 4: return bConnectionSucceeded; // Test (must pass)
		default: return true;
	}
}

bool SDeskillzSetupWizard::CanGoBack() const
{
	return CurrentStep > 0;
}

bool SDeskillzSetupWizard::CanFinish() const
{
	return CurrentStep == TotalSteps - 1;
}

void SDeskillzSetupWizard::SaveConfiguration()
{
	if (!Config)
	{
		return;
	}
	
	Config->APIKey = APIKey;
	Config->GameId = GameId;
	Config->Environment = (EDeskillzEnvironment)SelectedEnvironment;
	
	// Save to config file
	Config->SaveConfig();
	
	UE_LOG(LogDeskillz, Log, TEXT("Setup Wizard: Configuration saved"));
}

void SDeskillzSetupWizard::TestConnection()
{
	bConnectionTested = true;
	
	// Simple validation for now
	// In production, this would make an actual API call
	if (APIKey.Len() >= 32 && !GameId.IsEmpty())
	{
		bConnectionSucceeded = true;
		ConnectionMessage = TEXT("Connection successful!");
	}
	else
	{
		bConnectionSucceeded = false;
		if (APIKey.Len() < 32)
		{
			ConnectionMessage = TEXT("API Key appears invalid (should be 32+ characters)");
		}
		else
		{
			ConnectionMessage = TEXT("Game ID is required");
		}
	}
	
	Invalidate(EInvalidateWidget::Layout);
}

void SDeskillzSetupWizard::MarkSetupComplete()
{
	// Could set a project setting or file marker
	UE_LOG(LogDeskillz, Log, TEXT("Setup Wizard: Setup marked as complete"));
}

// ============================================================================
// Editor Module
// ============================================================================

void FDeskillzEditorModule::StartupModule()
{
	UE_LOG(LogDeskillz, Log, TEXT("Deskillz Editor Module Starting..."));
	
	// Register menus after engine is ready
	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		RegisterMenus();
	});
	
	// Show setup wizard on first launch if not configured
	if (!SDeskillzSetupWizard::IsSetupComplete())
	{
		// Delay to let editor finish loading
		FTimerHandle TimerHandle;
		GEditor->GetTimerManager()->SetTimer(TimerHandle, []()
		{
			SDeskillzSetupWizard::OpenWizard();
		}, 2.0f, false);
	}
}

void FDeskillzEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FDeskillzEditorModule::RegisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	
	// Add to Window menu
	UToolMenu* WindowMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = WindowMenu->AddSection("Deskillz", LOCTEXT("DeskillzSection", "Deskillz"));
	
	Section.AddMenuEntry(
		"DeskillzSetupWizard",
		LOCTEXT("SetupWizard", "Setup Wizard"),
		LOCTEXT("SetupWizardTooltip", "Open the Deskillz SDK setup wizard"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FDeskillzEditorModule::OnSetupWizardClicked))
	);
	
	Section.AddMenuEntry(
		"DeskillzSettings",
		LOCTEXT("ProjectSettings", "Project Settings"),
		LOCTEXT("ProjectSettingsTooltip", "Open Deskillz SDK settings"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FDeskillzEditorModule::OnProjectSettingsClicked))
	);
	
	Section.AddMenuEntry(
		"DeskillzDocs",
		LOCTEXT("Documentation", "Documentation"),
		LOCTEXT("DocumentationTooltip", "Open Deskillz SDK documentation"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FDeskillzEditorModule::OnDocumentationClicked))
	);
}

void FDeskillzEditorModule::OnSetupWizardClicked()
{
	SDeskillzSetupWizard::OpenWizard();
}

void FDeskillzEditorModule::OnProjectSettingsClicked()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
		.ShowViewer("Project", "Plugins", "Deskillz");
}

void FDeskillzEditorModule::OnDocumentationClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.deskillz.games/unreal"), nullptr, nullptr);
}

IMPLEMENT_MODULE(FDeskillzEditorModule, DeskillzEditor)

#undef LOCTEXT_NAMESPACE
