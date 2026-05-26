// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
	
public:
	UDebuffNiagaraComponent();
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DebuffTag;
	
private:
	virtual void BeginPlay() override;
	void OnDebuffTagChangedCallback(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);
	
};
