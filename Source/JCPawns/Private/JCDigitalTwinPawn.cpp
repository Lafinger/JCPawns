// Fill out your copyright notice in the Description page of Project Settings.


#include "JCDigitalTwinPawn.h"

#include "JCDigitalTwinPawnInputComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AJCDigitalTwinPawn::AJCDigitalTwinPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// scene
	SphereComponent = CreateDefaultSubobject<USphereComponent>(FName("DigitalTwinPawnSphere"));
	SphereComponent->InitSphereRadius(35.0f);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("DigitalTwinPawnSpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeTransform(FTransform::Identity);
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("DigitalTwinPawnCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->SetRelativeLocationAndRotation(FVector(0, 0, 0), FRotator(0, 0, 0));
	CameraComponent->bUsePawnControlRotation = false;
	
	JCDigitalTwinPawnInputComponent = CreateDefaultSubobject<UJCDigitalTwinPawnInputComponent>(FName("JCDigitalTwinPawnInput"));
}

// Called every frame
void AJCDigitalTwinPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FocusCurrentLocation.Equals(FocusTargetLocation, 0.05))
	{
		FocusCurrentLocation = FMath::InterpExpoOut(FocusStartLocation, FocusTargetLocation, 0.05f);
		FocusCurrentLocation = FMath::VInterpTo(FocusStartLocation, FocusTargetLocation, DeltaTime, FocusSpeed);
		FocusStartLocation = FocusCurrentLocation;
		SetActorLocation(FocusCurrentLocation);
	}
}


// Called when the game starts or when spawned
void AJCDigitalTwinPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// ActivateInput();
}

void AJCDigitalTwinPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AJCDigitalTwinPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent); // No bindings by default.

	JCDigitalTwinPawnInputComponent->SetupPlayerInputComponent(PlayerInputComponent);
}

void AJCDigitalTwinPawn::ActivateInput()
{
	JCDigitalTwinPawnInputComponent->Activate();
}

void AJCDigitalTwinPawn::DeactivateInput()
{
	JCDigitalTwinPawnInputComponent->Deactivate();
}

void AJCDigitalTwinPawn::FocusViewportOnActor(const AActor* InTargetActor)
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

void AJCDigitalTwinPawn::FocusViewportAsCamera(ACameraActor* InCameraActor)
{
	if(bIsBlending)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if(!PlayerController)
	{
		ensureAlwaysMsgf(false, TEXT("This pawn has not been possessed by PlayerController!"));
		return;
	}
	
	if(CameraComponent->GetComponentLocation().Equals(InCameraActor->GetActorLocation(), 10.0) && PlayerController->GetControlRotation().Equals(InCameraActor->GetActorRotation(), 5.0))
	{
		return;
	}

	// start to blend
	bIsBlending = true;
	DeactivateInput();
	PlayerController->SetViewTargetWithBlend(InCameraActor, BlendTime, VTBlend_EaseInOut, 1.0, false);
	DelegateHandle_BlendComplete = PlayerController->PlayerCameraManager->OnBlendComplete().AddWeakLambda(this, [this, InCameraActor, PlayerController]()
	{
		FVector StartPos = InCameraActor->GetActorLocation();
		FVector EndPos = InCameraActor->GetActorLocation() + InCameraActor->GetActorForwardVector() * LineTraceDistance;

		ECollisionChannel CollisionChannel = ECollisionChannel::ECC_Visibility;
		UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
		FHitResult OutHit;
		bool const bHit = World ? World->LineTraceSingleByChannel(OutHit, StartPos, EndPos, CollisionChannel) : false;
		if(bHit)
		{
			SetActorLocation(OutHit.Location);
			SpringArmComponent->TargetArmLength = OutHit.Distance;
			JCDigitalTwinPawnInputComponent->SpringArmLengthTemp = SpringArmComponent->TargetArmLength;
		}
		else
		{
			SetActorLocation(InCameraActor->GetActorLocation());
			SpringArmComponent->TargetArmLength = 0.0;
			JCDigitalTwinPawnInputComponent->SpringArmLengthTemp = SpringArmComponent->TargetArmLength;
		}
		
		PlayerController->Possess(this);
		PlayerController->SetControlRotation(
			FRotator(FMath::Clamp(InCameraActor->GetActorRotation().Pitch, -JCDigitalTwinPawnInputComponent->LookClamp.Y, -JCDigitalTwinPawnInputComponent->LookClamp.X),
				InCameraActor->GetActorRotation().Yaw,
				0.0));
		ActivateInput();
		PlayerController->PlayerCameraManager->OnBlendComplete().Remove(DelegateHandle_BlendComplete);
		bIsBlending = false;
	});
}

bool AJCDigitalTwinPawn::CalcCameraLocationWithBoundingBox(const FBox& InBoundingBox, FVector& OutTargetLocation)
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
		ensureAlwaysMsgf(false, TEXT("This pawn has not been possessed by PlayerController!"));
		return false;
	}

	float ViewFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
	const float HalfFOVRadians = FMath::DegreesToRadians(ViewFOV / 2.0f);
	const float DistanceFromSphere = Radius / FMath::Tan(HalfFOVRadians);
	FVector CameraOffsetVector = PlayerController->GetControlRotation().Vector() * -DistanceFromSphere;
	
	OutTargetLocation = Position + CameraOffsetVector;
	return true;
}

void AJCDigitalTwinPawn::LerpToTargetLocation(const FVector& InTargetLocation)
{
	FocusTargetLocation = InTargetLocation;
	FocusStartLocation = GetActorLocation();
}
