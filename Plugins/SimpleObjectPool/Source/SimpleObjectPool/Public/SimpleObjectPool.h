// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"

/** Module log category used by the pool subsystem and startup/shutdown code. */
DECLARE_LOG_CATEGORY_EXTERN(LogSimpleObjectPool, Log, All);

/** Minimal plugin module; runtime pool state is owned by UObjectPoolSubsystem per world. */
class FSimpleObjectPoolModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
