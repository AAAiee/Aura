// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleObjectPool.h"

#define LOCTEXT_NAMESPACE "FSimpleObjectPoolModule"

void FSimpleObjectPoolModule::StartupModule()
{
	// Module startup is intentionally lightweight; the world subsystem owns runtime pool state.
	UE_LOG(LogTemp, Warning, TEXT("SimpleObjectPool module has started!"));
}

void FSimpleObjectPoolModule::ShutdownModule()
{
	// Runtime pool cleanup is handled by UObjectPoolSubsystem::Deinitialize for each world.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSimpleObjectPoolModule, SimpleObjectPool)
