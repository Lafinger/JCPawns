#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JCDigitalTwinPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class UFloatingPawnMovement;
class UJCDigitalTwinPawnInputComponent;


UCLASS(BlueprintType)
class JCPAWNS_API AJCDigitalTwinPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AJCDigitalTwinPawn();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
public:
	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JC")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JC")
	USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JC")
	UCameraComponent* CameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JC")
	UJCDigitalTwinPawnInputComponent* JCDigitalTwinPawnInputComponent;


	UFUNCTION(BlueprintCallable, Category = "JC")
	void FocusViewportOnActor(const AActor* InTargetActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JC")
	float FocusSpeed = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JC")
	float LineTraceDistance = 1000000000.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JC")
	float BlendTime = 1.0;

private:
	bool CalcCameraLocationWithBoundingBox(const FBox& InBoundingBox, FVector& OutTargetLocation);
	
	void LerpToTargetLocation(const FVector& InTargetLocation);

	bool bIsBlending = false;

	/** Current viewport Position. */
	FVector	FocusCurrentLocation;

	/** Viewport start location when animating to another location */
	FVector FocusStartLocation;
	
	/** Desired viewport location when animating between two locations */
	FVector	FocusTargetLocation;
};
