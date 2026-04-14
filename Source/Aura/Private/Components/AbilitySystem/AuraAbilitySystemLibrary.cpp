// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "UI/HUD/AuraHUD.h"
#include "Player/AuraPlayerState.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

UAuraOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraAbilitySystemLibrary::GetOverlayWidgetController - WorldContextObject is null."));
		return nullptr;
	}

	// UI controllers are local-player objects, so we resolve through local player controller.
	APlayerController* LocalPlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld()) : nullptr;
	if (!LocalPlayerController)
	{
		return nullptr;
	}

	AAuraHUD* AuraHUD = Cast<AAuraHUD>(LocalPlayerController->GetHUD());
	AAuraPlayerState* PS = LocalPlayerController->GetPlayerState<AAuraPlayerState>();
	if (!AuraHUD || !PS)
	{
		return nullptr;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(PS->GetAttributeSet());
	if (!ASC || !AttributeSet)
	{
		return nullptr;
	}

	const FWidgetControllerParameters Params(LocalPlayerController, PS, ASC, AttributeSet);
	return AuraHUD->GetOverlayWidgetController(Params);
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraAbilitySystemLibrary::GetAttributeMenuWidgetController - WorldContextObject is null."));
		return nullptr;
	}

	APlayerController* LocalPlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld()) : nullptr;
	if (!LocalPlayerController)
	{
		return nullptr;
	}

	AAuraHUD* AuraHUD = Cast<AAuraHUD>(LocalPlayerController->GetHUD());
	AAuraPlayerState* PS = LocalPlayerController->GetPlayerState<AAuraPlayerState>();
	if (!AuraHUD || !PS)
	{
		return nullptr;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(PS->GetAttributeSet());
	if (!ASC || !AttributeSet)
	{
		return nullptr;
	}

	const FWidgetControllerParameters Params(LocalPlayerController, PS, ASC, AttributeSet);
	return AuraHUD->GetAttributeMenuWidgetController(Params);
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC, float Level)
{
	check(ASC);

	/*
	 * Default-attribute startup flow:
	 *   1. Resolve the avatar actor from the ASC.
	 *   2. Bail on clients because only the server is allowed to author Gameplay Effect state.
	 *   3. Read the class-info data asset from the authoritative GameMode.
	 *   4. Build the primary / secondary / vital specs.
	 *   5. Apply them to self so the replicated AttributeSet becomes the single source of truth.
	 *
	 * Keeping the flow here means both Aura-controlled pawns and enemy pawns can share the same
	 * initialization rules instead of duplicating startup effect code in multiple character classes.
	 */
	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!ensureMsgf(AvatarActor, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes requires ASC %s to have a valid avatar actor."), *GetNameSafe(ASC)))
	{
		return;
	}

	// Gameplay Effects are authoritative state, so only the server should seed default attributes.
	if (!AvatarActor->HasAuthority())
	{
		return;
	}


	UCharacterClassInfo* ClassInfos =  GetCharacterClassInfo(WorldContextObject);
	if (!ensureMsgf(ClassInfos, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes requires CharacterClassInfo to be assigned on %s."), *GetNameSafe(WorldContextObject)))
	{
		return;
	}

	const FCharacterClassDefaultInfo ClassInfoEntry = ClassInfos->GetDefaultInfoForClass(CharacterClass);
	FGameplayEffectContextHandle PrimaryEffectContextHandle = ASC->MakeEffectContext();
	PrimaryEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfoEntry.PrimaryAttributeEffect, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing a primary attribute effect for class %d."), static_cast<uint8>(CharacterClass)))
	{
		// Primary attributes are the class-specific branch of the setup flow.
		const FGameplayEffectSpecHandle PrimarySpecHandle = ASC->MakeOutgoingSpec(ClassInfoEntry.PrimaryAttributeEffect, Level, PrimaryEffectContextHandle);
		if (ensureMsgf(PrimarySpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the primary attribute spec for class %d."), static_cast<uint8>(CharacterClass)))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*PrimarySpecHandle.Data.Get());
		}
	}


	FGameplayEffectContextHandle SecondaryEffectContextHandle = ASC->MakeEffectContext();
	SecondaryEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfos->SecondaryAttributes, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing the shared secondary attribute effect.")))
	{
		// Secondary attributes are shared by every class, so they come from the common asset fields.
		const FGameplayEffectSpecHandle SecondarySpecHandle = ASC->MakeOutgoingSpec(ClassInfos->SecondaryAttributes, Level, SecondaryEffectContextHandle);
		if (ensureMsgf(SecondarySpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the secondary attribute spec.")))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SecondarySpecHandle.Data.Get());
		}
	}


	FGameplayEffectContextHandle VitalEffectContextHandle = ASC->MakeEffectContext();
	VitalEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfos->VitalAttributes, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing the shared vital attribute effect.")))
	{
		// Vital attributes are last so the final health / mana caps reflect any earlier startup effects.
		const FGameplayEffectSpecHandle VitalSpecHandle = ASC->MakeOutgoingSpec(ClassInfos->VitalAttributes, Level, VitalEffectContextHandle);
		if (ensureMsgf(VitalSpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the vital attribute spec.")))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*VitalSpecHandle.Data.Get());
		}
	}
}

void UAuraAbilitySystemLibrary::InitialzeDefaultAbilities(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC)
{
	check(ASC);
	(void)CharacterClass; // CommonAbilities are currently shared across classes; keep the parameter for future per-class expansion.

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!ensureMsgf(AvatarActor, TEXT("AuraAbilitySystemLibrary::InitialzeDefaultAbilities requires ASC %s to have a valid avatar actor."), *GetNameSafe(ASC)))
	{
		return;
	}

	// Ability grants are authoritative gameplay state, so make the rule explicit instead of relying
	// on the server-only GameMode lookup to fail on clients.
	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return;

	for (const TSubclassOf<UGameplayAbility> & AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		// These are the shared "always available" combat abilities that every spawned combatant
		// should own before moment-to-moment gameplay begins.
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// CharacterClassInfo lives on GameMode because it is server-authored setup data; callers that
	// run on clients should expect this lookup to return nullptr.
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!AuraGameMode) return nullptr;

	UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;

	return CharacterClassInfo;
}
