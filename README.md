# Aura - UE5 GAS Top-Down Action RPG Systems Prototype

Aura is a **gameplay systems prototype** built in **Unreal Engine 5** and **C++**. The project focuses on **action-RPG combat**, **Gameplay Ability System architecture**, **player-facing UI**, **enemy behavior**, and **reusable gameplay infrastructure**.

## Demo

Click the thumbnail below to watch the gameplay demo on YouTube.

<p align="center">
  <a href="https://youtu.be/y_uQL8ulQLg">
    <img src="Docs/Media/AuraDemo-thumbnail.jpg" alt="Aura gameplay demo" width="860">
  </a>
</p>

Direct link: [Aura Gameplay Systems Demo](https://youtu.be/y_uQL8ulQLg)

### Inventory System Progress

Click the thumbnail below to watch the inventory-system progress video on YouTube.

<p align="center">
  <a href="https://www.youtube.com/watch?v=ghusU05mUH0">
    <img src="https://img.youtube.com/vi/ghusU05mUH0/maxresdefault.jpg" alt="Replicated spatial inventory progress video" width="860">
  </a>
</p>

Direct link: [Replicated Spatial Inventory Update](https://www.youtube.com/watch?v=ghusU05mUH0)

## My Contributions

I implemented the core **C++ gameplay systems** shown in the demo, including **GAS-based abilities**, **damage calculation**, **spell UI flow**, **projectile behavior**, and **custom object pooling**. I also added several extensions beyond the tutorial baseline, including **draggable UI windows**, **confirmation-based spell assignment**, **pooled projectile reset behavior**, and an in-progress **replicated spatial inventory system**.

## Project Status

This is an **in-progress gameplay systems prototype**, not a shipped game. The current focus is building **clean, reusable gameplay architecture** and demonstrating **player-facing combat/UI systems**.

## Highlights

| Feature | What It Demonstrates | Source |
|---|---|---|
| **UE5 Gameplay Ability System** | `GameplayAbility`, `GameplayEffect`, `AttributeSet`, `GameplayTags`, **cooldowns**, **spell status**, and **spell progression** | [ASC](Source/Aura/Public/Components/AbilitySystem/AuraAbilitySystemComponent.h), [ASC cpp](Source/Aura/Private/Components/AbilitySystem/AuraAbilitySystemComponent.cpp), [AttributeSet](Source/Aura/Public/Components/AbilitySystem/AuraAttributeSet.h) |
| **Custom damage pipeline** | `GameplayEffectSpec`, `SetByCaller`, `ExecutionCalculation`, custom `GameplayEffectContext`, **resistances**, **block**, **crit**, **debuffs**, **knockback**, **XP** | [Damage ability base](Source/Aura/Private/Components/AbilitySystem/Ability/AuraDamageGameplayAbility.cpp), [Damage execution](Source/Aura/Private/Components/AbilitySystem/ExecCalc/ExecCalc_Damage.cpp), [Effect context](Source/Aura/Public/AuraAbilityTypes.h) |
| **MVC-style UMG architecture** | **Gameplay state** routed through `WidgetController` classes and **delegate-driven Blueprint widgets** | [Base controller](Source/Aura/Public/UI/WidgetController/AuraWidgetController.h), [Spell menu controller](Source/Aura/Private/UI/WidgetController/AuraSpellMenuWidgetController.cpp), [Overlay controller](Source/Aura/Private/UI/WidgetController/AuraOverlayWidgetController.cpp) |
| **Spell and attribute UI** | **Spell unlock**, **equip**, **level-up**, **input binding**, **attribute display**, and **confirmation-based assignment flow** | [Spell menu controller](Source/Aura/Private/UI/WidgetController/AuraSpellMenuWidgetController.cpp), [Attribute menu controller](Source/Aura/Private/UI/WidgetController/AttributeMenuWidgetController.cpp) |
| **Replicated spatial inventory** | **Category-based inventory grids**, **item pickup validation**, **stacking**, **drag/drop**, **item swapping**, server-authoritative placement, `FastArray` item replication, replicated grid state, and WidgetController-driven UMG rendering | [Inventory component](Plugins/InventorySystem/Source/InventorySystem/Public/InventoryManagement/Component/InvSS_InventoryComponent.h), [Grid manager](Plugins/InventorySystem/Source/InventorySystem/Public/InventoryManagement/Grid/InvSS_InventoryGridManager.h), [Inventory widget controller](Plugins/InventorySystem/Source/InventorySystem/Public/Widgets/WidgetController/InvSS_InventoryWidgetController.h), [Inventory grid](Plugins/InventorySystem/Source/InventorySystem/Public/Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid.h) |
| **Projectile gameplay** | **Moving projectiles**, **homing projectiles**, **multi-projectile FireBolt**, and **lightning propagation** | [FireBolt](Source/Aura/Private/Components/AbilitySystem/Ability/AuraFireBolt.cpp), [Electrocute](Source/Aura/Private/Components/AbilitySystem/Ability/AuraBeamSpell.cpp), [Projectile base](Source/Aura/Public/Effect/AuraProjectile.h) |
| **Reusable projectile pooling** | Custom `SimpleObjectPool` plugin for **replicated projectile reuse** and **pooled actor lifecycle** | [Pool subsystem](Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h), [Pool implementation](Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp), [Pooled gameplay actor](Source/Aura/Public/Effect/AuraPooledGameplayActor.h) |
| **Enemy behavior** | **Summon behavior**, **minion-gated re-summon logic**, and **FireBolt attack behavior** | [Summon ability](Source/Aura/Private/Components/AbilitySystem/Ability/AuraSummonAbility.cpp), [AI controller](Source/Aura/Public/AI/AuraAIController.h) |
| **Camera visibility** | Fades actors between the camera and player character to preserve **player visibility** | Source currently implemented through Blueprint/content setup |

## Technical Focus

### Gameplay Ability System

The core **combat** and **spell systems** are built around **GAS** concepts:

- **`AbilitySystemComponent`:** owns **ability specs**, handles **activation**, routes **input tags**, and updates **spell status**. ([header](Source/Aura/Public/Components/AbilitySystem/AuraAbilitySystemComponent.h), [cpp](Source/Aura/Private/Components/AbilitySystem/AuraAbilitySystemComponent.cpp))
- **`AttributeSet`:** stores **health**, **mana**, **attributes**, and **meta-attributes** such as incoming damage. ([header](Source/Aura/Public/Components/AbilitySystem/AuraAttributeSet.h), [cpp](Source/Aura/Private/Components/AbilitySystem/AuraAttributeSet.cpp))
- **`GameplayAbility`:** defines **spell behavior** such as FireBolt, Electrocute, targeting, projectile spawning, and chain logic. ([base](Source/Aura/Public/Components/AbilitySystem/Ability/AuraGameplayAbility.h), [FireBolt](Source/Aura/Private/Components/AbilitySystem/Ability/AuraFireBolt.cpp), [Electrocute](Source/Aura/Private/Components/AbilitySystem/Ability/AuraBeamSpell.cpp))
- **`GameplayEffect`:** applies **attribute changes**, **cooldowns**, **costs**, **debuffs**, **XP rewards**, and **level-up effects**. ([damage ability setup](Source/Aura/Private/Components/AbilitySystem/Ability/AuraDamageGameplayAbility.cpp), [attribute execution response](Source/Aura/Private/Components/AbilitySystem/AuraAttributeSet.cpp))
- **`ExecutionCalculation`:** centralizes **damage calculation** for armor, resistances, block, critical hits, debuffs, knockback, and impulses. ([damage execution](Source/Aura/Private/Components/AbilitySystem/ExecCalc/ExecCalc_Damage.cpp))
- **`GameplayEffectContext`:** carries **custom combat data** such as damage type, debuff info, knockback force, death impulse, and critical/block flags. ([context struct](Source/Aura/Public/AuraAbilityTypes.h), [serialization](Source/Aura/Public/AuraAbilityTypes.cpp))
- **`GameplayTags`:** drive **ability input**, **cooldown matching**, **spell status**, **ability type**, **message lookup**, and **UI state**. ([tag manager](Source/Aura/Public/AuraGameTagManager.h), [tag registration](Source/Aura/Private/AuraGameTagManager.cpp))

### UI Architecture

Aura uses a **WidgetController-based UI flow**:

- **Models:** **GAS**, `PlayerState`, `AttributeSet`, and data assets hold **gameplay state**. ([PlayerState](Source/Aura/Public/Player/AuraPlayerState.h), [AttributeSet](Source/Aura/Public/Components/AbilitySystem/AuraAttributeSet.h), [AbilityInfo data](Source/Aura/Public/Components/AbilitySystem/Data/AbilityInfo.h))
- **Controllers:** `WidgetController` classes translate gameplay state into **UI-facing events**. ([base controller](Source/Aura/Public/UI/WidgetController/AuraWidgetController.h), [spell menu](Source/Aura/Private/UI/WidgetController/AuraSpellMenuWidgetController.cpp), [attribute menu](Source/Aura/Private/UI/WidgetController/AttributeMenuWidgetController.cpp))
- **Views:** **UMG widgets** subscribe through **delegates** and update without directly owning gameplay logic. ([AuraUserWidget](Source/Aura/Public/UI/Widget/AuraUserWidget.h), [Spell menu widget](Source/Aura/Public/UI/Widget/AuraSpellMenuWidget.h), [Draggable window widget](Source/Aura/Public/UI/Widget/AuraDraggableWindowWidget.h))

This keeps **gameplay code**, **UI state**, and **widget presentation** separated.

### Inventory System Prototype

The inventory work is built as a separate **InventorySystem plugin** focused on replicated spatial item placement and UI separation:

- **Inventory component:** owns the replicated item list, validates pickup/drop requests on authority, and exposes local UI delegates. ([header](Plugins/InventorySystem/Source/InventorySystem/Public/InventoryManagement/Component/InvSS_InventoryComponent.h), [cpp](Plugins/InventorySystem/Source/InventorySystem/Private/InventoryManagement/Component/InvSS_InventoryComponent.cpp))
- **Grid manager:** owns per-category grid state, slot occupancy, parent-slot indices, stack counts, and replicated grid revisions. ([header](Plugins/InventorySystem/Source/InventorySystem/Public/InventoryManagement/Grid/InvSS_InventoryGridManager.h), [cpp](Plugins/InventorySystem/Source/InventorySystem/Private/InventoryManagement/Grid/InvSS_InventoryGridManager.cpp))
- **FastArray item replication:** replicates inventory item entries while the grid manager replicates spatial placement separately. ([FastArray header](Plugins/InventorySystem/Source/InventorySystem/Public/InventoryManagement/FastArray/InvSS_FastArray.h), [FastArray cpp](Plugins/InventorySystem/Source/InventorySystem/Private/InventoryManagement/FastArray/InvSS_FastArray.cpp))
- **WidgetController UI flow:** converts inventory/grid state into view data for UMG widgets, keeping gameplay state out of the visual grid. ([controller](Plugins/InventorySystem/Source/InventorySystem/Private/Widgets/WidgetController/InvSS_InventoryWidgetController.cpp), [grid render](Plugins/InventorySystem/Source/InventorySystem/Private/Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid_Render.cpp), [grid interaction](Plugins/InventorySystem/Source/InventorySystem/Private/Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid_Interaction.cpp))

Progress video: [Replicated Spatial Inventory Update](https://www.youtube.com/watch?v=ghusU05mUH0)

### Extensions Beyond Tutorial Scope

This project includes **custom extensions** intended to push the prototype beyond the base learning material:

- **Draggable and cached Attribute/Spell menu windows.** ([draggable widget](Source/Aura/Private/UI/Widget/AuraDraggableWindowWidget.cpp), [HUD window cache](Source/Aura/Private/UI/HUD/AuraHUD.cpp))
- **Confirmation flow** for spell assignment to avoid accidental input binding changes. ([spell menu controller](Source/Aura/Private/UI/WidgetController/AuraSpellMenuWidgetController.cpp))
- **Custom object pooling plugin** for reusable projectiles. ([pool subsystem](Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h), [pool implementation](Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp))
- **Pooled replicated projectile reset behavior** for collision, movement, homing, audio, and damage state. ([projectile base](Source/Aura/Public/Effect/AuraProjectile.h), [projectile cpp](Source/Aura/Private/Effect/AuraProjectile.cpp), [pooled actor base](Source/Aura/Public/Effect/AuraPooledGameplayActor.h))

## Tech Stack

- Unreal Engine 5
- C++
- Gameplay Ability System
- UMG
- GameplayTags
- Behavior Trees
- Replicated inventory architecture
- Object pooling
- Git / Git LFS
- Rider / Visual Studio

## Running the Project

- Requires Unreal Engine 5.
- Large Unreal assets are tracked with Git LFS.
- Open `Aura.uproject` after cloning and syncing LFS assets.

## Planned Work

- Continue inventory stack transfer, swap rules, and drag/drop polish.
- Broader multiplayer support beyond the current replicated projectile systems.
- More enemy archetypes and ability interactions.
- Additional gameplay video breakdowns and feature-specific clips.
