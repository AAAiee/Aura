# Source Cleanup Checklist

Generated: 2026-05-20

## Scope

Changed C++ source and header files since the last commit:

- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/SimpleObjectPool.cpp`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/SimpleObjectPool.h`
- `Source/Aura/Private/AuraGameTagManager.cpp`
- `Source/Aura/Private/Character/AuraCharacter.cpp`
- `Source/Aura/Private/Character/AuraCharacterBase.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraDamageGameplayAbility.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraFireBolt.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraGameplayAbility.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`
- `Source/Aura/Private/Components/AbilitySystem/AuraAbilitySystemComponent.cpp`
- `Source/Aura/Private/Components/AbilitySystem/AuraAbilitySystemLibrary.cpp`
- `Source/Aura/Private/Components/AbilitySystem/AuraAttributeSet.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Data/AbilityInfo.cpp`
- `Source/Aura/Private/Components/AbilitySystem/ExecCalc/ExecCalc_Damage.cpp`
- `Source/Aura/Private/Effect/AuraProjectile.cpp`
- `Source/Aura/Private/Game/AuraGameInstance.cpp`
- `Source/Aura/Private/Player/AuraPlayerController.cpp`
- `Source/Aura/Private/UI/HUD/AuraHUD.cpp`
- `Source/Aura/Private/UI/Widget/AuraAttributeMenuWidget.cpp`
- `Source/Aura/Private/UI/Widget/AuraSpellMenuWidget.cpp`
- `Source/Aura/Private/UI/WidgetController/AttributeMenuWidgetController.cpp`
- `Source/Aura/Private/UI/WidgetController/AuraOverlayWidgetController.cpp`
- `Source/Aura/Private/UI/WidgetController/AuraSpellMenuWidgetController.cpp`
- `Source/Aura/Private/UI/WidgetController/AuraWidgetController.cpp`
- `Source/Aura/Public/AuraAbilityTypes.cpp`
- `Source/Aura/Public/AuraAbilityTypes.h`
- `Source/Aura/Public/AuraGameTagManager.h`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraFireBolt.h`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraGameplayAbility.h`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraProjectileSpell.h`
- `Source/Aura/Public/Components/AbilitySystem/AuraAbilitySystemComponent.h`
- `Source/Aura/Public/Components/AbilitySystem/AuraAbilitySystemLibrary.h`
- `Source/Aura/Public/Components/AbilitySystem/AuraAttributeSet.h`
- `Source/Aura/Public/Components/AbilitySystem/Data/AbilityInfo.h`
- `Source/Aura/Public/Components/AbilitySystem/ExecCalc/ExecCalc_Damage.h`
- `Source/Aura/Public/Effect/AuraProjectile.h`
- `Source/Aura/Public/Game/AuraGameInstance.h`
- `Source/Aura/Public/Game/AuraGameModeBase.h`
- `Source/Aura/Public/Player/AuraPlayerController.h`
- `Source/Aura/Public/Player/AuraPlayerState.h`
- `Source/Aura/Public/UI/HUD/AuraHUD.h`
- `Source/Aura/Public/UI/Widget/AuraAttributeMenuWidget.h`
- `Source/Aura/Public/UI/Widget/AuraDraggableWindowWidget.h`
- `Source/Aura/Public/UI/Widget/AuraSpellMenuWidget.h`
- `Source/Aura/Public/UI/WidgetController/AuraOverlayWidgetController.h`
- `Source/Aura/Public/UI/WidgetController/AuraSpellMenuWidgetController.h`
- `Source/Aura/Public/UI/WidgetController/AuraWidgetController.h`

## Official References Reviewed

- Epic C++ Coding Standard for Unreal Engine: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- Include What You Use for Unreal Engine: https://dev.epicgames.com/documentation/en-us/unreal-engine/include-what-you-use-iwyu-for-unreal-engine-programming
- Unreal Engine Modules and dependency guidance: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules
- Unreal Build Tool target IWYU checks: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-build-tool-target-reference

Local UE 5.6 GameplayAbilities source reviewed:

- `C:/Program Files/Epic Games/UE_5.6/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h`
- `C:/Program Files/Epic Games/UE_5.6/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/Abilities/GameplayAbility.cpp`
- `C:/Program Files/Epic Games/UE_5.6/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h`
- `C:/Program Files/Epic Games/UE_5.6/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/AbilitySystemComponent.cpp`

## UE5 Cleanup Rules Reviewed And Enriched

- Keep every `.cpp` file's matching header as the first include. This mirrors UE IWYU guidance and lets non-unity/PCH-disabled builds prove the header owns its dependencies.
- Keep each Unreal `.generated.h` include last in its header include block, with no includes after it.
- Prefer Include What You Use: remove clearly unused headers, avoid monolithic headers, and add direct includes for reflected structs, value types, base classes, and delegate payloads used in a header.
- Prefer forward declarations in headers for pointer/reference-only UObject or actor types, then include their definitions in the `.cpp` that dereferences them.
- Do not explicitly include PCH headers. Let Unreal Build Tool force PCHs as an optimization layer.
- Keep Public headers limited to the dependencies required by other modules. Put dependencies used only by implementation code in Private `.cpp` includes or `PrivateDependencyModuleNames`.
- Use GAS public accessors that match current engine style, such as `GetDynamicSpecSourceTags()` and `GetAssetTags()`, instead of reaching into deprecated or version-sensitive public members.
- When iterating or mutating activatable ability specs, use `FScopedAbilityListLock` where the code may traverse the ASC ability list.
- Keep ability status/input/slot tags as dynamic spec source tags, while ability identity tags stay authored on the GameplayAbility asset.
- For custom GAS `FGameplayEffectContext` payloads, keep `Duplicate()` and `NetSerialize()` in sync with every custom field because effect contexts are copied and serialized outside normal UObject property replication.
- Prefer small educational comments above lifecycle boundaries and cross-system handoffs: authority checks, effect context metadata, meta attributes, widget-controller broadcasts, and pooled actor borrow/return transitions.
- Preserve UPROPERTY/UFUNCTION ordering when moving it could alter Blueprint/editor-facing layout; prefer section comments, direct includes, or local formatting over risky reshuffling.
- Make `.cpp` definition order match the header declaration order when the move is mechanically safe and does not split related authority/client RPC flow.
- Use comments that explain gameplay intent, authority rules, data ownership, replication assumptions, or editor-facing meaning; remove placeholder, stale, or "what the next line does" comments.
- Keep logging categories module-specific. Avoid `LogTemp` in cleaned code when a project or plugin log category exists.
- Keep cleanup behavior-neutral by default: no gameplay value changes, tag string changes, Blueprint property renames, authority rewrites, or flow rewrites. If cleanup exposes a tiny correctness/safety issue in newly changed code, fix it only when the intent is already clear and document the reason in the surrounding comment.
- Verify with `git diff --check`, header/include sanity checks, and an Unreal build when available.

## Planned Cleanup Items

- Remove trailing whitespace and placeholder comments from changed C++ files.
- Tighten IWYU includes for new ability, spell-menu, HUD, and widget-controller code.
- Keep `.cpp` matching headers first and `.generated.h` includes last.
- Normalize spacing around Unreal macros, delegates, tag checks, and GAS helper methods.
- Clarify comments around ability status tags, spell-menu selection, widget-controller data ownership, and projectile/pool logging without changing runtime behavior.
- Add teaching comments to currently uncommented changed sections, especially GAS context serialization, debuff application, AttributeSet meta-attribute consumption, ASC spell-equipping flow, and widget-controller UI broadcasts.
- Group header methods by lifecycle/interface/function area so related overrides, public commands, RPCs, delegates, and private helpers are easy to review later.
- Avoid touching binary assets, IDE files, or unrelated generated plugin files.
