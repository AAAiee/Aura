#pragma once

#include "CoreMinimal.h"
#include "ControllerDelegates.generated.h"


/*Dynamic Multicast Delegates — Blueprint widgets bind to these to receive attribute updates.
 * Keeping shared signatures in one header avoids re-declaring the same delegate types across
 * multiple widget controllers and makes the UI event surface easier to keep consistent.*/
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangeSignature, float, NewValue);
