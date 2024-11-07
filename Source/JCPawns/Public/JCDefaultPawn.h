// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "JCDefaultPawn.generated.h"

UCLASS()
class JCPAWNS_API AJCDefaultPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AJCDefaultPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "JC")
	void FocusViewportOnActor(const AActor* InTargetActor);

private:
	bool CalcCameraLocationWithBoundingBox(const FBox& InBoundingBox, FVector& OutTargetLocation);
	
	void LerpToTargetLocation(const FVector& InTargetLocation);

	/** Current viewport Position. */
	FVector	CurrentLocation;
	/** Desired viewport location when animating between two locations */
	FVector	TargetLocation;
	/** Viewport start location when animating to another location */
	FVector StartLocation;
};
