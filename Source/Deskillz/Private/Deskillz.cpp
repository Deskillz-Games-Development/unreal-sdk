// Copyright Deskillz Games. All Rights Reserved.

#include "Deskillz.h"
#include "Core/DeskillzConfig.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FDeskillzModule"

DEFINE_LOG_CATEGORY(LogDeskillz);

void FDeskillzModule::StartupModule()
{
	UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Module Starting..."));
	
	RegisterSettings();
	
	UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Module Started Successfully"));
}

void FDeskillzModule::ShutdownModule()
{
	UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Module Shutting Down..."));
	
	UnregisterSettings();
	
	UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Module Shutdown Complete"));
}

void FDeskillzModule::RegisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"Plugins",
			"Deskillz",
			LOCTEXT("RuntimeSettingsName", "Deskillz SDK"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Deskillz competitive gaming SDK"),
			GetMutableDefault<UDeskillzConfig>()
		);
	}
#endif
}

void FDeskillzModule::UnregisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Deskillz");
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDeskillzModule, Deskillz)
