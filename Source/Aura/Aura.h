// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git

#pragma once

#include "CoreMinimal.h"

/**
 * Module-level header for the Aura game module.
 * Put project-wide constants and macros here so every file can access them via #include "Aura/Aura.h".
 */

/** Stencil value used with Custom Depth to render a red outline for highlightable targets. */
#define CUSTOM_DEPTH_RED 250

/** Collision channel reserved for Aura projectile actors and traces. */
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1

/** Object channel assigned to enemy capsules for gameplay-specific filtering. */
#define ECC_EnemyCollision ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Target ECollisionChannel::ECC_GameTraceChannel3
