// @Copyright HaolunYuan


#include "Effect/AuraActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

AAuraActor::AAuraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(StaticMeshComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Sphere");
	SphereComponent->SetupAttachment(GetRootComponent());
}

void AAuraActor::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraActor::OnOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AAuraActor::OverlapEnd); 
}

void AAuraActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtrherAcotr,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtrherAcotr))
	{ 
		
		//TODO: Using Game Effect Instead, this is just a hack 
		const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass()));

		UAuraAttributeSet* MutableAttributeSets = const_cast<UAuraAttributeSet*> (AuraAttributeSet);
		MutableAttributeSets->SetHealth(AuraAttributeSet->GetHealth() + 2.f);
		Destroy(); 
	}
}

void AAuraActor::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

