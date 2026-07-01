// Fill out your copyright notice in the Description page of Project Settings.


#include "JCDigitalTwinPawn.h"

#include "JCDigitalTwinPawnInputComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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
	ClearFocusCompletionCallbacks(Cast<APlayerController>(GetController()));
	FocusViewportAsCameraFinishedDelegate.Clear();

	if(IsValid(BlendSnapshotCamera))
	{
		BlendSnapshotCamera->Destroy();
		BlendSnapshotCamera = nullptr;
	}

	bIsBlending = false;

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

void AJCDigitalTwinPawn::FocusViewportAsCamera(ACameraActor* InCameraActor, const float InBlendTime, const EViewTargetBlendFunction InBlendFunction, const float InBlendExp, bool bInLockOutgoing, FJCFocusViewportAsCameraFinishedDelegate InOnFinished)
{
	if(!InCameraActor)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if(!PlayerController || !PlayerController->PlayerCameraManager)
	{
		ensureAlwaysMsgf(false, TEXT("This pawn has not been possessed by PlayerController!"));
		return;
	}

	if(!bIsBlending && IsViewportAlreadyFocusedOnCamera(InCameraActor, PlayerController))
	{
		if(InOnFinished.IsBound())
		{
			InOnFinished.Execute();
		}
		return;
	}

	const int32 FocusRequestId = ++FocusViewportAsCameraRequestId;
	const float ResolvedBlendTime = InBlendTime == -1 ? BlendTime : InBlendTime;

	ClearFocusCompletionCallbacks(PlayerController);
	FocusViewportAsCameraFinishedDelegate = InOnFinished;
	if(ResolvedBlendTime > 0.0f && CaptureCurrentCameraPOV(PlayerController))
	{
		PlayerController->SetViewTarget(BlendSnapshotCamera);
	}

	// start to blend
	bIsBlending = true;
	StopFocusLocationLerp();
	DeactivateInput();

	if(ResolvedBlendTime <= 0.0f)
	{
		PlayerController->SetViewTarget(InCameraActor);
		CompleteFocusViewportAsCamera(InCameraActor, PlayerController, FocusRequestId);
		return;
	}

	PlayerController->SetViewTargetWithBlend(InCameraActor, ResolvedBlendTime, InBlendFunction, InBlendExp, bInLockOutgoing);
	const TWeakObjectPtr<ACameraActor> CameraActorWeak(InCameraActor);
	const TWeakObjectPtr<APlayerController> PlayerControllerWeak(PlayerController);
	DelegateHandle_BlendComplete = PlayerController->PlayerCameraManager->OnBlendComplete().AddWeakLambda(this, [this, CameraActorWeak, PlayerControllerWeak, FocusRequestId]()
	{
		ACameraActor* CameraActor = CameraActorWeak.Get();
		APlayerController* PlayerController = PlayerControllerWeak.Get();
		if(!IsValid(CameraActor) || !IsValid(PlayerController))
		{
			AbortFocusViewportAsCamera(PlayerController, FocusRequestId);
			return;
		}

		CompleteFocusViewportAsCamera(CameraActor, PlayerController, FocusRequestId);
	});

	if(UWorld* World = GetWorld())
	{
		FTimerDelegate FallbackDelegate = FTimerDelegate::CreateWeakLambda(this, [this, CameraActorWeak, PlayerControllerWeak, FocusRequestId]()
		{
			ACameraActor* CameraActor = CameraActorWeak.Get();
			APlayerController* PlayerController = PlayerControllerWeak.Get();
			if(!IsValid(CameraActor) || !IsValid(PlayerController))
			{
				AbortFocusViewportAsCamera(PlayerController, FocusRequestId);
				return;
			}

			CompleteFocusViewportAsCamera(CameraActor, PlayerController, FocusRequestId);
		});
		World->GetTimerManager().SetTimer(TimerHandle_FocusCompletionFallback, FallbackDelegate, ResolvedBlendTime + 0.05f, false);
	}
}

void AJCDigitalTwinPawn::StopFocusLocationLerp()
{
	FocusCurrentLocation = GetActorLocation();
	FocusStartLocation = FocusCurrentLocation;
	FocusTargetLocation = FocusCurrentLocation;
}

void AJCDigitalTwinPawn::ClearBlendCompleteDelegate(APlayerController* InPlayerController)
{
	if(DelegateHandle_BlendComplete.IsValid())
	{
		if(InPlayerController && InPlayerController->PlayerCameraManager)
		{
			InPlayerController->PlayerCameraManager->OnBlendComplete().Remove(DelegateHandle_BlendComplete);
		}
		DelegateHandle_BlendComplete.Reset();
	}
}

void AJCDigitalTwinPawn::ClearFocusCompletionTimer()
{
	if(TimerHandle_FocusCompletionFallback.IsValid())
	{
		if(UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle_FocusCompletionFallback);
		}
		TimerHandle_FocusCompletionFallback.Invalidate();
	}
}

void AJCDigitalTwinPawn::ClearFocusCompletionCallbacks(APlayerController* InPlayerController)
{
	ClearBlendCompleteDelegate(InPlayerController);
	ClearFocusCompletionTimer();
}

ACameraActor* AJCDigitalTwinPawn::GetOrCreateBlendSnapshotCamera()
{
	if(IsValid(BlendSnapshotCamera))
	{
		return BlendSnapshotCamera;
	}

	UWorld* World = GetWorld();
	if(!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	BlendSnapshotCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParameters);
	if(BlendSnapshotCamera)
	{
		BlendSnapshotCamera->SetActorHiddenInGame(true);
		BlendSnapshotCamera->SetActorEnableCollision(false);
		BlendSnapshotCamera->SetActorTickEnabled(false);
	}

	return BlendSnapshotCamera;
}

bool AJCDigitalTwinPawn::CaptureCurrentCameraPOV(APlayerController* InPlayerController)
{
	if(!InPlayerController || !InPlayerController->PlayerCameraManager)
	{
		return false;
	}

	ACameraActor* SnapshotCamera = GetOrCreateBlendSnapshotCamera();
	if(!SnapshotCamera)
	{
		return false;
	}

	UCameraComponent* SnapshotCameraComponent = SnapshotCamera->GetCameraComponent();
	if(!SnapshotCameraComponent)
	{
		return false;
	}

	const FMinimalViewInfo& CurrentPOV = InPlayerController->PlayerCameraManager->GetCameraCacheView();
	SnapshotCamera->SetActorLocationAndRotation(CurrentPOV.Location, CurrentPOV.Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	SnapshotCameraComponent->SetFieldOfView(CurrentPOV.FOV);
	SnapshotCameraComponent->SetFirstPersonFieldOfView(CurrentPOV.FirstPersonFOV);
	SnapshotCameraComponent->SetFirstPersonScale(CurrentPOV.FirstPersonScale);
	SnapshotCameraComponent->SetEnableFirstPersonFieldOfView(CurrentPOV.bUseFirstPersonParameters);
	SnapshotCameraComponent->SetEnableFirstPersonScale(CurrentPOV.bUseFirstPersonParameters);
	SnapshotCameraComponent->SetOrthoWidth(CurrentPOV.OrthoWidth);
	SnapshotCameraComponent->SetAutoCalculateOrthoPlanes(CurrentPOV.bAutoCalculateOrthoPlanes);
	SnapshotCameraComponent->SetAutoPlaneShift(CurrentPOV.AutoPlaneShift);
	SnapshotCameraComponent->SetOrthoNearClipPlane(CurrentPOV.OrthoNearClipPlane);
	SnapshotCameraComponent->SetOrthoFarClipPlane(CurrentPOV.OrthoFarClipPlane);
	SnapshotCameraComponent->SetUpdateOrthoPlanes(CurrentPOV.bUpdateOrthoPlanes);
	SnapshotCameraComponent->SetUseCameraHeightAsViewTarget(CurrentPOV.bUseCameraHeightAsViewTarget);
	SnapshotCameraComponent->SetAspectRatio(CurrentPOV.AspectRatio);
	SnapshotCameraComponent->SetConstraintAspectRatio(CurrentPOV.bConstrainAspectRatio);
	SnapshotCameraComponent->bOverrideAspectRatioAxisConstraint = CurrentPOV.AspectRatioAxisConstraint.IsSet();
	if(CurrentPOV.AspectRatioAxisConstraint.IsSet())
	{
		SnapshotCameraComponent->SetAspectRatioAxisConstraint(CurrentPOV.AspectRatioAxisConstraint.GetValue());
	}
	SnapshotCameraComponent->SetUseFieldOfViewForLOD(CurrentPOV.bUseFieldOfViewForLOD);
	SnapshotCameraComponent->SetProjectionMode(CurrentPOV.ProjectionMode);
	SnapshotCameraComponent->SetOverscan(CurrentPOV.GetOverscan());
	SnapshotCameraComponent->SetPostProcessBlendWeight(CurrentPOV.PostProcessBlendWeight);
	SnapshotCameraComponent->PostProcessSettings = CurrentPOV.PostProcessSettings;

	return true;
}

bool AJCDigitalTwinPawn::IsViewportAlreadyFocusedOnCamera(ACameraActor* InCameraActor, APlayerController* InPlayerController) const
{
	if(!IsValid(InCameraActor) || !InPlayerController || !InPlayerController->PlayerCameraManager)
	{
		return false;
	}

	if(InPlayerController->PlayerCameraManager->GetViewTarget() != this)
	{
		return false;
	}

	const FMinimalViewInfo& CurrentPOV = InPlayerController->PlayerCameraManager->GetCameraCacheView();
	return CurrentPOV.Location.Equals(InCameraActor->GetActorLocation(), 10.0)
		&& CurrentPOV.Rotation.Equals(InCameraActor->GetActorRotation(), 5.0);
}

void AJCDigitalTwinPawn::AbortFocusViewportAsCamera(APlayerController* InPlayerController, int32 InFocusRequestId)
{
	if(InFocusRequestId != FocusViewportAsCameraRequestId)
	{
		return;
	}

	APlayerController* PlayerController = InPlayerController ? InPlayerController : Cast<APlayerController>(GetController());
	ClearFocusCompletionCallbacks(PlayerController);
	StopFocusLocationLerp();
	bIsBlending = false;
	FocusViewportAsCameraFinishedDelegate.Clear();
	ActivateInput();
}

void AJCDigitalTwinPawn::CompleteFocusViewportAsCamera(ACameraActor* InCameraActor, APlayerController* InPlayerController, int32 InFocusRequestId)
{
	if(InFocusRequestId != FocusViewportAsCameraRequestId)
	{
		return;
	}

	if(!IsValid(InCameraActor) || !IsValid(InPlayerController))
	{
		AbortFocusViewportAsCamera(InPlayerController, InFocusRequestId);
		return;
	}

	ClearFocusCompletionCallbacks(InPlayerController);

	FVector StartPos = InCameraActor->GetActorLocation();
	FVector EndPos = InCameraActor->GetActorLocation() + InCameraActor->GetActorForwardVector() * LineTraceDistance;

	ECollisionChannel CollisionChannel = ECollisionChannel::ECC_Visibility;
	UWorld* World = GetWorld();
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

	StopFocusLocationLerp();
	InPlayerController->Possess(this);
	InPlayerController->SetControlRotation(
		FRotator(FMath::Clamp(InCameraActor->GetActorRotation().Pitch, -JCDigitalTwinPawnInputComponent->LookClamp.Y, -JCDigitalTwinPawnInputComponent->LookClamp.X),
			InCameraActor->GetActorRotation().Yaw,
			0.0));
	ActivateInput();
	bIsBlending = false;

	FJCFocusViewportAsCameraFinishedDelegate FinishedDelegate = FocusViewportAsCameraFinishedDelegate;
	FocusViewportAsCameraFinishedDelegate.Clear();
	if(FinishedDelegate.IsBound())
	{
		FinishedDelegate.Execute();
	}
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
