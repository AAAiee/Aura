# Pooling Implementation Notes

## Goal

Document the object-pooling work in a compact, accurate way so later steps can continue without re-discovering the design decisions.

## Current Status

Completed:
- Step 1: harden the `SimpleObjectPool` plugin so Aura can safely build projectile/effect pooling on top of it.
- Step 2: add an Aura-side pooled gameplay base actor that bridges the plugin lifecycle into Aura code.
- Step 3: convert `AAuraProjectile` so it can safely participate in the pool lifecycle.
- Step 4: wire `UAuraProjectileSpell` so the ability can initialize the pool, borrow a projectile, and launch it.
- Step 5: add a plugin-side object pool config data asset type for class-based pool tuning.
- Step 6: add plugin developer settings so the project can assign a shared pool config asset cleanly.
- Step 7: clean up projectile spell pool bootstrap to use config-driven initialization and recycle policy.

Not started yet:
- pooled transient effect actor integration

## Design Direction

Two-layer ownership model:

- `SimpleObjectPool` plugin:
  - owns pool storage
  - owns borrow/return flow
  - owns auto-return timer tracking
  - owns lifecycle notifications for pooled actors
- Aura:
  - owns gameplay state
  - owns projectile/effect initialization payloads
  - owns hit handling and state reset rules
  - decides which actor types should actually be pooled

Reason:
- keeps the plugin generic and reusable
- keeps Aura-specific GAS/combat logic out of shared infrastructure

## Step 1 Changes

### 1. Pool subsystem scope changed

File:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`

Change:
- `UObjectPoolSubsystem` now derives from `UWorldSubsystem` instead of `UGameInstanceSubsystem`

Reason:
- pooled actors should belong to a world/session
- avoids carrying pooled state across worlds or map transitions

### 2. Added pooled actor lifecycle interface

File:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/PoolableActor.h`

Added:
- `UPoolableActor`
- `IPoolableActor`
- hooks:
  - `OnTakenFromPool()`
  - `OnReturnedToPool()`

Reason:
- pooled gameplay actors need a reusable place to reset state on borrow/return
- generic plugin activation/deactivation is not enough for projectiles/effect actors

### 3. Added timer tracking for auto-return

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Added:
- `TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveReturnTimers`
- `ClearReturnTimer(AActor*)`

Behavior:
- any pending return timer is cleared before an actor is reused
- any pending return timer is cleared when an actor is manually returned

Reason:
- without this, an old timer can fire after an actor has already been re-borrowed
- that would cause incorrect mid-flight returns for projectiles

### 4. Added reverse lookup for return safety

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Added:
- `TMap<TWeakObjectPtr<AActor>, TSubclassOf<AActor>> ActorToPoolClassMap`

Reason:
- returning an actor no longer depends only on `Actor->GetClass()`
- safer and faster lookup when sending an actor back to its pool

### 5. Borrow/return flow now notifies pooled actors

File:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Behavior:
- `ActivateActor()` calls `NotifyActorTakenFromPool()`
- `DeactivateActor()` calls `NotifyActorReturnedToPool()`

Reason:
- this is the extension point Aura will use for resetting movement, collision, hit state, GAS handles, visuals, timers, and similar runtime state

### 6. Pool warmup is silent

File:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Change:
- `DeactivateActor()` now supports `bNotifyPoolableActor`
- initial prewarmed actors are deactivated with `false`

Reason:
- actors created during pool initialization should not receive a fake `OnReturnedToPool()` before ever being used

### 7. Added build-facing cleanup

File:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/SimpleObjectPool.Build.cs`

Change:
- moved core runtime modules into public dependencies
- kept `AIModule`, `Slate`, `SlateCore` as private dependencies

Reason:
- the plugin public headers expose engine/core types and should compile cleanly when consumed by Aura

## Public API After Step 1

`UObjectPoolSubsystem` now exposes:

- `bool IsPoolInitialized(TSubclassOf<AActor> ActorClass) const`
- `void InitializePool(TSubclassOf<AActor> ActorClass, int32 InitialSize)`
- `AActor* GetPooledActor(...)`
- `template<typename T> T* GetPooledActorTyped(...)`
- `void ReturnActorToPool(AActor* Actor)`

New actor extension point:

- implement `IPoolableActor`
- override:
  - `OnTakenFromPool()`
  - `OnReturnedToPool()`

## Step 2 Changes

### 1. Added Aura pooled base actor

Files:
- `Source/Aura/Public/Effect/AuraPooledGameplayActor.h`
- `Source/Aura/Private/Effect/AuraPooledGameplayActor.cpp`

Added class:
- `AAuraPooledGameplayActor : public AActor, public IPoolableActor`

Responsibilities:
- provide a single Aura base for transient pooled gameplay actors
- bridge the plugin callbacks into Aura overridable hooks
- provide a standard `RequestReturnToPool()` entry point

### 2. Added Aura-facing lifecycle hooks

`AAuraPooledGameplayActor` implements the plugin callbacks:

- `OnTakenFromPool()`
- `OnReturnedToPool()`

and translates them into Aura hooks:

- `HandleTakenFromPool()`
- `ResetPooledState()`
- `HandleReturnedToPool()`

Call order:

- borrow:
  - `OnTakenFromPool()`
  - set `bIsActiveInPool = true`
  - call `HandleTakenFromPool()`
- return:
  - `OnReturnedToPool()`
  - call `ResetPooledState()`
  - set `bIsActiveInPool = false`
  - call `HandleReturnedToPool()`

Reason:
- subclasses can reset gameplay state without knowing anything about the plugin internals
- this keeps projectile/effect subclasses focused on their own state only

### 3. Added self-return helper

Method:
- `RequestReturnToPool()`

Behavior:
- gets the owning `UObjectPoolSubsystem` from the world
- returns `this` actor to the pool
- logs a warning if no valid pool subsystem is available

Reason:
- later pooled actors can shut themselves down cleanly on hit/timeout/end-of-life
- gameplay code does not need to duplicate pool lookup logic everywhere

### 4. Added pooled active-state flag

Method:
- `bool IsActiveInPool() const`

Stored state:
- `bIsActiveInPool`

Reason:
- useful for subclasses that need to guard overlap/timer logic
- makes later projectile code easier to reason about

## Step 3 Changes

### 1. Projectile now derives from the pooled Aura base

Files:
- `Source/Aura/Public/Effect/AuraProjectile.h`
- `Source/Aura/Private/Effect/AuraProjectile.cpp`

Change:
- `AAuraProjectile` now inherits from `AAuraPooledGameplayActor`

Reason:
- projectiles now have access to the standard Aura pool lifecycle and self-return helper

### 2. Added projectile pool lifecycle behavior

Added overrides:
- `HandleTakenFromPool()`
- `ResetPooledState()`

Borrow behavior:
- clear hit flag
- re-enable overlap generation on the collision component
- ensure projectile movement uses the collision component as its updated component
- stop any leftover movement and reactivate movement component ticking

Return behavior:
- clear hit flag
- stop projectile movement
- deactivate the movement component
- disable overlap generation
- disable collision on the collision component

Reason:
- pooled projectiles must scrub per-use runtime state between shots
- this is the first concrete use of the plugin + Aura pooled lifecycle design

### 3. Added overlap-driven pool return

Added runtime state:
- `bHasRegisteredHit`
- `bReturnToPoolOnAnyOverlap`

Updated `OnProjectileOverlap()` behavior:
- ignore overlap if projectile is not currently active in the pool
- ignore duplicate hit processing
- ignore self/owner overlaps
- mark hit as consumed
- call `RequestReturnToPool()`

Reason:
- pooled projectiles should shut down by returning to the pool, not by being treated like disposable actors
- duplicate overlaps need to be guarded once the first hit has already triggered return

## Step 4 Changes

### 1. Projectile ability now borrows from the pool

Files:
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraProjectileSpell.h`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`

Added ability properties:
- `ProjectileClass`
- `InitialPoolSize`
- `ProjectileRecycleDelay`
- `ProjectileSpawnForwardOffset`

Added helper methods:
- `GetProjectileSpawnTransform(...)`
- `EnsureProjectilePool(...)`

Server activation flow now:
- validate actor info and projectile class
- require authority
- get world pool subsystem
- lazily initialize the projectile pool if needed
- borrow a typed `AAuraProjectile`
- set owner/instigator
- launch the projectile
- end the ability

Reason:
- this is the first full end-to-end use of the updated pooling system from gameplay code

### 2. Added minimal projectile launch helper

Files:
- `Source/Aura/Public/Effect/AuraProjectile.h`
- `Source/Aura/Private/Effect/AuraProjectile.cpp`

Added:
- `LaunchInDirection(const FVector& Direction)`

Behavior:
- normalize launch direction
- rotate actor to face the direction
- set the projectile movement updated component
- assign velocity from direction * initial speed
- update movement component velocity

Also updated return reset to clear:
- owner
- instigator

Reason:
- a borrowed projectile needs an explicit launch step to become useful
- owner/instigator should not leak across pooled uses

## Step 5 Changes

### 1. Added plugin-side pool config data asset type

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolConfigDataAsset.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolConfigDataAsset.cpp`

Added:
- `FPoolClassConfig`
- `UObjectPoolConfigDataAsset`

`FPoolClassConfig` currently stores:
- `ActorClass`
- `InitialPoolSize`
- `DefaultRecycleDelay`

`UObjectPoolConfigDataAsset` currently stores:
- `TArray<FPoolClassConfig> PoolClassConfigs`
- helper lookup:
  - `FindPoolConfigByClass(...)`

Reason:
- pool sizing/configuration is better represented as reusable system data than as spell-local tuning
- placing the asset type in the plugin keeps pooling-related types together while still allowing the project to create asset instances in the editor

### 2. Deferred runtime hookup on purpose

Not implemented yet:
- how the subsystem discovers the asset instance

Possible later hookup points:
- subsystem soft object path
- project settings
- GameMode
- Aura bootstrap object

Reason:
- we want the config type available now without prematurely locking the runtime discovery mechanism

## Step 6 Changes

### 1. Added plugin developer settings for pool config asset assignment

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolDeveloperSettings.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolDeveloperSettings.cpp`

Added:
- `UObjectPoolDeveloperSettings`
- `TSoftObjectPtr<UObjectPoolConfigDataAsset> DefaultPoolConfig`

Editor result:
- project can assign the shared pool config asset through Project Settings under Plugins

Reason:
- this is cleaner than making a Blueprint subsystem just to hold a config reference
- keeps the subsystem implementation generic while still allowing project-level asset assignment

### 2. Added subsystem-side config loading helpers

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Added methods:
- `GetPoolConfig()`
- `InitializePoolFromConfig(...)`
- `LoadPoolConfigIfNeeded()`

Behavior:
- subsystem lazily loads the assigned pool config asset from developer settings
- subsystem caches the loaded asset
- subsystem can now initialize a pool for a class using config entries when available

Reason:
- this keeps configuration lookup inside the pooling system instead of pushing asset-loading logic into gameplay abilities

## Step 7 Changes

### 1. Removed manual pool size from projectile spell

Files:
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraProjectileSpell.h`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`

Removed from spell:
- `InitialPoolSize`
- `ProjectileRecycleDelay`

Reason:
- these values are now owned by the shared pool config asset rather than the ability class

### 2. Projectile spell now uses config-driven pool initialization

Changed:
- `EnsureProjectilePool(...)` now calls `InitializePoolFromConfig(ProjectileClass)`
- spell verifies the pool is actually initialized before trying to borrow

Reason:
- pool sizing/bootstrap is now system data, not spell-local setup

### 3. Projectile spell now reads recycle policy from config

Added helper:
- `GetProjectileRecyclePolicy(...)`

Behavior:
- default to manual return (`false`, `0.f`)
- if the shared pool config has a matching entry with `bUseDefaultRecycleDelay = true`
  - enable auto-return
  - use `DefaultRecycleDelay`

Reason:
- this removes the old spell-owned recycle delay and makes recycle behavior configurable per pooled class

### 4. Pool borrowing API is now config-driven by default

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`

Changed:
- `GetPooledActor(...)` now resolves pool config and recycle policy inside the subsystem
- `GetPooledActorTyped(...)` now only requires:
  - actor class
  - spawn transform
- added `GetPooledActorWithRecyclePolicy(...)` as the explicit override path when a caller truly needs a runtime policy override

Reason:
- keeps gameplay callers clean
- keeps config lookup inside the pooling system where it belongs
- preserves one explicit escape hatch for non-default borrowing behavior

### 5. Added fail-fast config validation and pool state logging

Files:
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`

Changed:
- added a shared config lookup helper to remove duplicate config-search code
- missing config asset / missing class config now surfaces through stronger `ensure` / `check` paths instead of being quietly ignored
- added borrow/return/init logs that report:
  - actor class
  - actor instance name
  - in-use count
  - total pool size

Reason:
- pool misconfiguration should fail loudly
- pool behavior is easier to debug when borrow/return state is visible in logs

## Validation Performed

Build command used:

```powershell
& 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' AuraEditor Win64 Development 'C:\Unreal5\Aura\Aura.uproject' -WaitMutex -NoHotReloadFromIDE
```

Observed result:
- earlier validation:
  - UnrealHeaderTool completed successfully
  - plugin and project source compiled
  - build failed at link step only because `UnrealEditor.exe` had the DLLs locked
- latest validation after Step 3:
  - full build succeeded
  - UHT succeeded
  - plugin compiled and linked
  - Aura compiled and linked

What this means:
- Step 1 through Step 3 are validated by a successful Unreal build

## Known Limitations After Step 6

Still intentionally not handled:

- projectile still uses a minimal launch contract instead of a full per-shot init payload
- projectile overlap currently only returns to pool; it does not yet apply gameplay effect or damage payload
- no effect actor currently borrows from the pool
- no replicated/network pooling behavior was added

Also deferred on purpose:

- pool config asset/data-driven warmup
- max pool size
- pool stats/debug UI
- stale invalid pooled actor scrubbing beyond current validity checks

## Next Recommended Step

Step 5 should expand projectile runtime data:

- source actor / instigator
- launch direction or target data
- lifetime tuning
- gameplay effect or damage payload
- movement initialization on launch

Then Step 6 should wire actual projectile gameplay:

- apply effects or damage on overlap
- optionally support friendly-fire / self-hit filtering rules
- add timeout behavior if desired beyond generic auto-return

## Files Modified So Far

- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolSubsystem.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolSubsystem.cpp`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/PoolableActor.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/SimpleObjectPool.Build.cs`
- `Source/Aura/Public/Effect/AuraPooledGameplayActor.h`
- `Source/Aura/Private/Effect/AuraPooledGameplayActor.cpp`
- `Source/Aura/Public/Effect/AuraProjectile.h`
- `Source/Aura/Private/Effect/AuraProjectile.cpp`
- `Source/Aura/Public/Components/AbilitySystem/Ability/AuraProjectileSpell.h`
- `Source/Aura/Private/Components/AbilitySystem/Ability/AuraProjectileSpell.cpp`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolConfigDataAsset.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolConfigDataAsset.cpp`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Public/ObjectPoolDeveloperSettings.h`
- `Plugins/SimpleObjectPool/Source/SimpleObjectPool/Private/ObjectPoolDeveloperSettings.cpp`

## Practical Reminder Before Next Build

If a future full build fails at link with locked DLLs, first check whether Unreal Editor is still holding:

- `UnrealEditor-SimpleObjectPool.dll`
- `UnrealEditor-Aura.dll`
