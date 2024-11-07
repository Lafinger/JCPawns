// Fill out your copyright notice in the Description page of Project Settings.


#include "JCDefaultPawn.h"

// Sets default values
AJCDefaultPawn::AJCDefaultPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AJCDefaultPawn::BeginPlay()
{
	Super::BeginPlay();

	CurrentLocation = GetActorLocation();
}

// Called every frame
void AJCDefaultPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentLocation.Equals(TargetLocation, 0.05))
	{
		CurrentLocation = FMath::InterpExpoOut(StartLocation, TargetLocation, 0.02f);
		StartLocation = CurrentLocation;
		SetActorLocation(CurrentLocation);
	}
}

// Called to bind functionality to input
void AJCDefaultPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AJCDefaultPawn::FocusViewportOnActor(const AActor* InTargetActor)
{
	// Create a bounding volume of InActor.
	FBox BoundingBox(ForceInit);
	if (USceneComponent* Root = InTargetActor->GetRootComponent())
	{
		TArray<USceneComponent*> SceneComponents;
		Root->GetChildrenComponents(true, SceneComponents);
		SceneComponents.Add(Root);

		bool bHasAtLeastOnePrimitiveComponent = false;
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(SceneComponent);
			if (PrimitiveComponent && PrimitiveComponent->IsRegistered())
			{
				// Some components can have huge bounds but are not visible.  Ignore these components unless it is the only component on the actor 
				const bool bIgnore = SceneComponents.Num() > 1 && PrimitiveComponent->GetIgnoreBoundsForEditorFocus();
				if (!bIgnore)
				{
					BoundingBox += PrimitiveComponent->Bounds.GetBox();
					bHasAtLeastOnePrimitiveComponent = true;
				}
			}
		}

		if (!bHasAtLeastOnePrimitiveComponent)
		{
			BoundingBox += Root->GetComponentLocation();
		}
	}

	FVector Location;
	bool IsCalcCameraToBoxSuccess = CalcCameraLocationWithBoundingBox(BoundingBox, Location);
	if(!IsCalcCameraToBoxSuccess)
	{
		return;
	}
	
	LerpToTargetLocation(Location);
}

bool AJCDefaultPawn::CalcCameraLocationWithBoundingBox(const FBox& InBoundingBox, FVector& OutTargetLocation)
{
	const FVector Position = InBoundingBox.GetCenter();
	float Radius = FMath::Max(InBoundingBox.GetExtent().Size(), 10.f);

	float AspectRatio = 0;
	FVector2D ViewportSize;
	GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
	if (ViewportSize.X > 0 && ViewportSize.Y > 0)
	{
		AspectRatio = ViewportSize.X / ViewportSize.Y;
	}
	if (AspectRatio > 1.0f)
	{
		Radius *= AspectRatio;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if(!PlayerController)
	{
		ensureAlwaysMsgf(false, TEXT("There has no player to controll!"));
		return false;
	}

	float ViewFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
	const float HalfFOVRadians = FMath::DegreesToRadians(ViewFOV / 2.0f);
	const float DistanceFromSphere = Radius / FMath::Tan(HalfFOVRadians);
	FVector CameraOffsetVector = PlayerController->GetControlRotation().Vector() * -DistanceFromSphere;
	
	OutTargetLocation = Position + CameraOffsetVector;
	return true;
}

void AJCDefaultPawn::LerpToTargetLocation(const FVector& InTargetLocation)
{
	TargetLocation = InTargetLocation;
	StartLocation = GetActorLocation();
}

