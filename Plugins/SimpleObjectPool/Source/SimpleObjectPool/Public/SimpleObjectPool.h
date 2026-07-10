// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"

/** Module log category used by the pool subsystem and startup/shutdown code. */
DECLARE_LOG_CATEGORY_EXTERN(LogSimpleObjectPool, Log, All);

/**
 * FSimpleObjectPoolModule
 *
 * Registers the SimpleObjectPool runtime module and its log category.
 *
 * Runtime pool state intentionally lives in UObjectPoolSubsystem per world, so
 * module startup and shutdown stay lightweight.
 *
 * Important functions:
 *   - StartupModule() - Emits module startup diagnostics.
 *   - ShutdownModule() - Leaves world-specific cleanup to UObjectPoolSubsystem.
 */
class FSimpleObjectPoolModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
