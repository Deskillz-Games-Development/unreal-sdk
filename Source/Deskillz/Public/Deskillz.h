// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDeskillz, Log, All);

/**
 * Deskillz SDK Module
 * 
 * Main module for the Deskillz competitive gaming platform SDK.
 * Provides tournament play, cryptocurrency prizes, and real-time multiplayer.
 */
class FDeskillzModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/**
	 * Singleton-like access to this module's interface.
	 * Beware of calling this during the shutdown phase - module might be unloaded.
	 *
	 * @return The singleton instance, loading the module on demand if needed
	 */
	static inline FDeskillzModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FDeskillzModule>("Deskillz");
	}
	
	/**
	 * Checks if this module is loaded and ready.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Deskillz");
	}

private:
	/** Handle to the registered settings */
	void RegisterSettings();
	void UnregisterSettings();
};
