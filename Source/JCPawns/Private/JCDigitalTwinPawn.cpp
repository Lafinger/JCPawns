// Fill out your copyright notice in the Description page of Project Settings.


#include "JCDigitalTwinPawn.h"

#include "JCDigitalTwinPawnInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AJCDigitalTwinPawn::AJCDigitalTwinPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnName = FName("DigitalTwinPawn");

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
}


// Called when the game starts or when spawned
void AJCDigitalTwinPawn::BeginPlay()
{
	Super::BeginPlay();
	
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
