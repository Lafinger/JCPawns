// Fill out your copyright notice in the Description page of Project Settings.


#include "JCDigitalTwinPawnInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "JCDigitalTwinPawn.h"
#include "JCEnhancedInputComponent.h"
#include "JCPawnsGameplayTags.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UJCDigitalTwinPawnInputComponent::UJCDigitalTwinPawnInputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void UJCDigitalTwinPawnInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
	if(!OwnerPawn)
	{
		return;
	}
	
	OwnerPawn->SpringArmComponent->TargetArmLength = UKismetMathLibrary::FInterpTo(
		OwnerPawn->SpringArmComponent->TargetArmLength,
		SpringArmLengthTemp,
		DeltaTime,
		LagSpeed
		);
}


// Called when the game starts
void UJCDigitalTwinPawnInputComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UJCDigitalTwinPawnInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UJCDigitalTwinPawnInputComponent::SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent)
{
	OwnerPawn = GetPawnChecked<AJCDigitalTwinPawn>();

	if(!OwnerPawn)
	{
		return;
	}
	
	if(APlayerController* PlayerController = GetController<APlayerController>())
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
			SpringArmLengthTemp = OwnerPawn->SpringArmComponent->TargetArmLength;
			FRotator PawnRotator = OwnerPawn->GetActorRotation();
			PlayerController->SetControlRotation(FRotator(PawnRotator.Pitch, PawnRotator.Yaw, 0));
			OwnerPawn->SetActorRotation(FRotator(0.0, 0.0, 0.0), ETeleportType::TeleportPhysics);
		}
	}

	if(UJCEnhancedInputComponent* JCEnhancedInputComponent = CastChecked<UJCEnhancedInputComponent>(InPlayerInputComponent))
	{
		JCEnhancedInputComponent->BindJCInputAction(JCInputConfig, JCPawnsGameplayTags::JC_Input_GroundMove, ETriggerEvent::Triggered, this, &ThisClass::OnInputEvent_GroundMove);
		JCEnhancedInputComponent->BindJCInputAction(JCInputConfig, JCPawnsGameplayTags::JC_Input_ViewMove, ETriggerEvent::Triggered, this, &ThisClass::OnInputEvent_ViewMove);
		JCEnhancedInputComponent->BindJCInputAction(JCInputConfig, JCPawnsGameplayTags::JC_Input_Look, ETriggerEvent::Triggered, this, &ThisClass::OnInputEvent_Look);
		JCEnhancedInputComponent->BindJCInputAction(JCInputConfig, JCPawnsGameplayTags::JC_Input_Zoom, ETriggerEvent::Triggered, this, &ThisClass::OnInputEvent_Zoom);
	}
	
}

float UJCDigitalTwinPawnInputComponent::GetZoomFactor()
{
	return UKismetMathLibrary::NormalizeToRange(OwnerPawn->SpringArmComponent->TargetArmLength, ZoomClamp.X, ZoomClamp.Y);
}

void UJCDigitalTwinPawnInputComponent::OnInputEvent_GroundMove_Implementation(const FInputActionValue& InputActionValue)
{
	if(!OwnerPawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	APlayerController* PlayerController = GetController<APlayerController>();
	FRotator ZRotator =  UKismetMathLibrary::MakeRotator(0.0, 0.0, PlayerController->GetControlRotation().Yaw);
	
	float TempCurveFloat = 1.0;
	if(GroundMoveCurve)
	{
		float ZoomFactor = GetZoomFactor();
		TempCurveFloat = GroundMoveCurve->GetFloatValue(ZoomFactor);
	}
	
	if (Value.X != 0.0f)
	{
		FVector RightVector = UKismetMathLibrary::GetRightVector(ZRotator);
		FVector RightDeltaVector = RightVector * Value.X * GroundMoveSpeed * TempCurveFloat;
		OwnerPawn->AddActorLocalOffset(RightDeltaVector);
	}
	if (Value.Y != 0.0f)
	{
		FVector ForwardVector = UKismetMathLibrary::GetForwardVector(ZRotator);
		FVector ForwardDeltaVector = ForwardVector * Value.Y * GroundMoveSpeed * TempCurveFloat;
		OwnerPawn->AddActorLocalOffset(ForwardDeltaVector);
	}
	
}

void UJCDigitalTwinPawnInputComponent::OnInputEvent_ViewMove_Implementation(const FInputActionValue& InputActionValue)
{
	if(!OwnerPawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	APlayerController* PlayerController = GetController<APlayerController>();
	FRotator ZRotator =  PlayerController->GetControlRotation();
	
	if (Value.X != 0.0f)
	{
		FVector RightVector = UKismetMathLibrary::GetRightVector(ZRotator);
		FVector RightDeltaVector = RightVector * Value.X * ViewMoveSpeed;
		OwnerPawn->AddActorLocalOffset(RightDeltaVector);
	}
	if (Value.Y != 0.0f)
	{
		FVector UpVector = UKismetMathLibrary::GetUpVector(ZRotator);
		FVector UpDeltaVector = UpVector * Value.Y * ViewMoveSpeed;
		OwnerPawn->AddActorLocalOffset(UpDeltaVector);
	}
}

void UJCDigitalTwinPawnInputComponent::OnInputEvent_Look_Implementation(const FInputActionValue& InputActionValue)
{
	if(!OwnerPawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	APlayerController* PlayerController = GetController<APlayerController>();
	FRotator ControlRotation = PlayerController->GetControlRotation();
	
	float Pitch = ControlRotation.Pitch * LookVerticalSpeed + Value.Y;
	float PitchClamp = UKismetMathLibrary::FClamp(Pitch, -LookClamp.Y,-LookClamp.X);
	float Yaw = ControlRotation.Yaw * LookHorizontalSpeed + Value.X;
	PlayerController->SetControlRotation(FRotator(PitchClamp, Yaw, ControlRotation.Roll));
}

void UJCDigitalTwinPawnInputComponent::OnInputEvent_Zoom_Implementation(const FInputActionValue& InputActionValue)
{
	if(!OwnerPawn)
	{
		return;
	}
	
	const float AxisValue = InputActionValue.Get<float>();

	float TempCurveFloat = 1.0;
	if(ZoomCurve)
	{
		float ZoomFactor = GetZoomFactor();
		TempCurveFloat = ZoomCurve->GetFloatValue(ZoomFactor);
	}
	
	float ArmLength = AxisValue * ZoomSpeed * TempCurveFloat + SpringArmLengthTemp;
	float ArmLengthClamp = UKismetMathLibrary::FClamp(ArmLength, ZoomClamp.X,ZoomClamp.Y);
	SpringArmLengthTemp = ArmLengthClamp;
}
