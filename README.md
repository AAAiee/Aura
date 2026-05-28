# Aura - UE5 GAS Top-Down Action RPG Systems Prototype

Aura is a gameplay systems prototype built in Unreal Engine 5 and C++. The project focuses on action-RPG combat, Gameplay Ability System architecture, player-facing UI, enemy behavior, and reusable gameplay infrastructure.

## Demo

Click the thumbnail below to open the compressed gameplay demo.

<p align="center">
  <a href="Docs/Media/AuraDemo-preview.mp4">
    <img src="Docs/Media/AuraDemo-thumbnail.jpg" alt="Aura gameplay demo" width="860">
  </a>
</p>

## Highlights

- UE5 Gameplay Ability System implementation with GameplayAbilities, GameplayEffects, AttributeSets, GameplayTags, cooldowns, and spell progression.
- Custom damage pipeline using GameplayEffectSpecs, SetByCaller values, ExecutionCalculation, and custom GameplayEffectContext data.
- MVC-style UMG architecture where gameplay state is routed through WidgetControllers and delegates into Blueprint widgets.
- Spell and attribute UI, including movable menu windows, spell equip flow, automatic input binding, and confirmation-based assignment.
- Projectile gameplay including moving projectiles, homing projectiles, multi-projectile FireBolt, and lightning propagation to nearby enemies.
- Custom SimpleObjectPool plugin used to manage reusable replicated projectiles and reduce repeated actor spawning.
- Enemy behavior examples including summon logic, minion-gated re-summon flow, and FireBolt attack behavior.
- Camera visibility support that fades actors between the camera and Aura.

## Technical Focus

### Gameplay Ability System

The core combat and spell systems are built around GAS concepts:

- `AbilitySystemComponent` for ability ownership and activation.
- `AttributeSet` and meta-attributes for health, mana, incoming damage, and progression.
- `GameplayAbility` classes for spell behavior.
- `GameplayEffect` and `ExecutionCalculation` for damage, resistances, block, critical hits, debuffs, knockback, XP, and level-up effects.
- `GameplayTags` for input routing, cooldowns, spell status, ability type, and UI-driven state.

### UI Architecture

Aura uses a WidgetController-based UI flow:

- GAS, PlayerState, AttributeSet, and data assets hold gameplay state.
- WidgetControllers translate gameplay state into UI events.
- UMG widgets subscribe through delegates and update without directly owning gameplay logic.

This keeps gameplay code, UI state, and widget presentation separated.

### Extensions Beyond Tutorial Scope

This project includes custom extensions intended to push the prototype beyond the base learning material:

- Draggable and cached Attribute/Spell menu windows.
- Confirmation flow for spell assignment to avoid accidental input binding changes.
- Custom object pooling plugin for reusable projectiles.
- Pooled replicated projectile reset behavior for collision, movement, homing, audio, and damage state.
- FireBolt multi-projectile and homing behavior.
- Lightning spell propagation to nearby enemies.
- Actor fade system for camera visibility.
- Enemy re-summon logic that waits until existing minions are defeated.

## Tech Stack

- Unreal Engine 5
- C++
- Gameplay Ability System
- UMG
- GameplayTags
- Behavior Trees
- Object pooling
- Git / Git LFS
- Rider / Visual Studio

## Planned Work

- Inventory system.
- Expanded multiplayer replication support.
- More enemy archetypes and ability interactions.
- Additional gameplay video breakdowns and feature-specific clips.
