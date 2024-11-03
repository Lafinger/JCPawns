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

private:
	FName PawnName;
	
};
