// Copyright Víctor Muñoz Sept2017.

#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

/// Look for attached Physics Handle
void UGrabber::FindPhysicsHandleComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)	
	{ 
		UE_LOG(LogTemp, Error, TEXT("There was no PhysicsHandle found in: %s"), *GetOwner()->GetName())
	}
}

/// Setup attached Input Component
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		/// Bind the input action
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("There was no InputComponent found in: %s"), *GetOwner()->GetName());
	}
}

// Release physics handle
void UGrabber::Release()
{
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }
	// if physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		// move the object that we're holding each frame
		PhysicsHandle->SetTargetLocation(GetLineTrace().v2);
	}
}

// Grab physics handle
void UGrabber::Grab()
{
	/// Line-trace and see if reach any actors with PhysicsBody collision channel set
	UPrimitiveComponent* ComponentToGrab = GetHitResult().GetComponent();
	AActor* ActorHit = GetHitResult().GetActor();

	/// If we hit something then attach a physics handle
	if (ActorHit)
	{
		if (!PhysicsHandle) { return; }
		// Attach physics handle
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation(),
			ComponentToGrab->GetOwner()->GetActorRotation() //allow rotation
		);
	}
}

const FHitResult UGrabber::GetHitResult()
{
	/// Line-trace (ray-cast) out to reach distance
	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		GetLineTrace().v1,
		GetLineTrace().v2,
		ECC_PhysicsBody,
		FCollisionQueryParams(FName(TEXT("")), false, GetOwner()) // Setup query parameters
	);
	return Hit;
}

FTwoVectors UGrabber::GetLineTrace()
{
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	return FTwoVectors(PlayerViewPointLocation, PlayerViewPointLocation + (PlayerViewPointRotation.Vector() * Reach));
}