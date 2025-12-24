// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UDeskillzConfig;

/**
 * Deskillz Setup Wizard
 * 
 * Step-by-step guided setup window for integrating Deskillz SDK.
 * Opens automatically on first launch, or via Window > Deskillz > Setup Wizard
 * 
 * Steps:
 * 1. Welcome & Requirements check
 * 2. Enter API Key & Game ID
 * 3. Select Environment (Sandbox/Production)
 * 4. Configure Match Settings
 * 5. Test Connection
 * 6. Complete!
 */
class DESKILLZEDITOR_API SDeskillzSetupWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDeskillzSetupWizard) {}
	SLATE_END_ARGS()
	
	/** Construct the widget */
	void Construct(const FArguments& InArgs);
	
	/** Open the wizard window */
	static void OpenWizard();
	
	/** Check if setup is complete */
	static bool IsSetupComplete();
	
private:
	// ========================================================================
	// Wizard State
	// ========================================================================
	
	/** Current step (0-5) */
	int32 CurrentStep = 0;
	
	/** Total number of steps */
	static const int32 TotalSteps = 6;
	
	/** Step titles */
	static const TArray<FString> StepTitles;
	
	/** Config being edited */
	UDeskillzConfig* Config = nullptr;
	
	/** Entered API Key */
	FString APIKey;
	
	/** Entered Game ID */
	FString GameId;
	
	/** Selected environment */
	int32 SelectedEnvironment = 1; // Sandbox default
	
	/** Connection test result */
	bool bConnectionTested = false;
	bool bConnectionSucceeded = false;
	FString ConnectionMessage;
	
	// ========================================================================
	// UI Building
	// ========================================================================
	
	TSharedRef<SWidget> BuildStepContent();
	TSharedRef<SWidget> BuildWelcomeStep();
	TSharedRef<SWidget> BuildCredentialsStep();
	TSharedRef<SWidget> BuildEnvironmentStep();
	TSharedRef<SWidget> BuildSettingsStep();
	TSharedRef<SWidget> BuildTestStep();
	TSharedRef<SWidget> BuildCompleteStep();
	
	TSharedRef<SWidget> BuildNavigationButtons();
	TSharedRef<SWidget> BuildProgressIndicator();
	
	// ========================================================================
	// Actions
	// ========================================================================
	
	FReply OnBackClicked();
	FReply OnNextClicked();
	FReply OnFinishClicked();
	FReply OnTestConnectionClicked();
	FReply OnOpenPortalClicked();
	
	bool CanGoNext() const;
	bool CanGoBack() const;
	bool CanFinish() const;
	
	void SaveConfiguration();
	void TestConnection();
	void MarkSetupComplete();
};

/**
 * Deskillz Editor Module
 */
class FDeskillzEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	void RegisterMenus();
	void OnSetupWizardClicked();
	void OnProjectSettingsClicked();
	void OnDocumentationClicked();
	
	TSharedPtr<FExtender> MenuExtender;
};
