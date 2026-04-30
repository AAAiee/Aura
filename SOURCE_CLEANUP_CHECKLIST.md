# Source Cleanup Checklist

Generated: 2026-04-30

## Scope

Changed source files since the last commit:

- `Source/Aura/Private/AuraGameTagManager.cpp`
- `Source/Aura/Private/Character/AuraCharacterBase.cpp`
- `Source/Aura/Private/Character/AuraEnemy.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraDamageGameplayAbility.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`
- `Source/Aura/Private/Components/AbilitySystem/AuraAbilitySystemLibrary.cpp`
- `Source/Aura/Public/AuraGameTagManager.h`
- `Source/Aura/Public/Character/AuraCharacterBase.h`
- `Source/Aura/Public/Character/AuraEnemy.h`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraProjectileSpell.h`
- `Source/Aura/Public/Components/AbilitySystem/AuraAbilitySystemLibrary.h`
- `Source/Aura/Public/Interaction/CombatInterface.h`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraSummonAbility.cpp`
- `Source/Aura/Private/Interaction/Summonable.cpp`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraSummonAbility.h`
- `Source/Aura/Public/Interaction/Summonable.h`

## UE5 Cleanup Rules Reviewed

- Keep every `.cpp` file's matching header as the first include.
- Keep each Unreal `.generated.h` include last in its header include block.
- Prefer Include What You Use: remove clearly unused headers, avoid monolithic headers, and add direct forward declarations/includes only where they make dependencies clearer.
- Preserve UPROPERTY/UFUNCTION ordering when moving it could alter Blueprint/editor-facing layout; prefer section comments over risky reshuffling.
- Make `.cpp` definition order match the header declaration order when the move is mechanically safe.
- Use comments that explain gameplay intent, authority rules, data ownership, or editor-facing meaning; remove placeholder/stale comments.
- Keep code behavior unchanged: no logic fixes, value changes, signature changes, or gameplay flow rewrites during this cleanup.

Official references:

- Epic C++ Coding Standard for Unreal Engine: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- Include What You Use for Unreal Engine: https://dev.epicgames.com/documentation/en-us/unreal-engine/include-what-you-use-iwyu-for-unreal-engine-programming
- Unreal Engine Modules and dependency guidance: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules
- Unreal Build Tool target IWYU checks: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-build-tool-target-reference

## Planned Cleanup Items

- `AuraGameTagManager`: tighten ability/combat-socket/montage tag grouping, remove extra blank space, and make registration comments mirror the header groups.
- `AuraCharacterBase`: order includes cleanly, add missing forward declarations for newly referenced asset types, polish death/blood/minion comments, and fix stale/typo comments.
- `AuraEnemy`: organize interface methods so implementation order follows the header, normalize UPROPERTY spacing, and clarify summon/blackboard/death comments.
- `AuraProjectileSpell`: remove stale unused includes after the combat socket parameter change, keep include order IWYU-friendly, and trim empty activation noise comments.
- `CombatInterface`: replace placeholder comments with useful interface/struct documentation, add missing forward declarations, remove redundant forward declarations, and trim blank lines.
- `AuraSummonAbility`: replace generated placeholder comments, remove unused headers, add direct dependencies/forward declarations as needed, and document summon authoring parameters.
- `Summonable`: replace generated placeholder comments with a real interface description and keep the `.cpp` default implementation comment useful.
- `AuraAbilitySystemLibrary` and `AuraDamageGameplayAbility`: only touch comments/formatting if needed because current tracked changes appear to be line-ending/status noise rather than content diff.
