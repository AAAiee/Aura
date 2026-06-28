// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInvSS, Log, All);

/**
 * Module entry point for the InventorySystem plugin.
 */
class FInventorySystemModule : public IModuleInterface
{
public:
	/* IModuleInterface begins */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/* IModuleInterface ends */
};
