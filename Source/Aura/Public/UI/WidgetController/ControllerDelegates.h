#pragma once

#include "CoreMinimal.h"
#include "ControllerDelegates.generated.h"

/* Dynamic multicast delegates that Blueprint widgets bind to for controller-driven UI updates.
 * Keeping shared signatures in one header avoids re-declaring the same delegate types across
 * multiple widget controllers and makes the UI event surface easier to keep consistent. */
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangeSignature, float, NewValue);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);

