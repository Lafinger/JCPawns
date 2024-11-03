// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JCDigitalTwinPawnInputComponent.generated.h"

class UInputMappingContext;
class UJCInputConfig;
class AJCDigitalTwinPawn;
struct FInputActionValue;

// UCLASS(ClassGroup=(JC))
UCLASS(ClassGroup=(JC), meta=(BlueprintSpawnableComponent))
class JCPAWNS_API UJCDigitalTwinPawnInputComponent final: public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UJCDigitalTwinPawnInputComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to GetPawn must be derived from APawn");
		return Cast<T>(GetOwner());
	}

	template <class T>
	T* GetPawnChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to GetPawnChecked must be derived from APawn");
		return CastChecked<T>(GetOwner());
	}

	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' template parameter to GetController must be derived from AController");
		return GetPawnChecked<APawn>()->GetController<T>();
	}

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent);

	// Pawn input : Camera
	UFUNCTION(BlueprintNativeEvent, Category = "JC")
	void OnInputEvent_GroundMove(const FInputActionValue& InputActionValue);
	
	UFUNCTION(BlueprintNativeEvent, Category = "JC")
	void OnInputEvent_ViewMove(const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintNativeEvent, Category = "JC")
	void OnInputEvent_Look(const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintNativeEvent, Category = "JC")
	void OnInputEvent_Zoom(const FInputActionValue& InputActionValue);
	
	// Input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	TObjectPtr<UJCInputConfig> JCInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float LagSpeed = 4.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float ViewMoveSpeed = 50.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float GroundMoveSpeed = 50.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	TObjectPtr<UCurveFloat> GroundMoveCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float LookHorizontalSpeed = 1.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float LookVerticalSpeed = 1.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	FVector2D LookClamp = FVector2D(-89.0, 89.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	float ZoomSpeed = 100.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	TObjectPtr<UCurveFloat> ZoomCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JC")
	FVector2D ZoomClamp = FVector2D(10.0, 10000.0);

private:
	float GetZoomFactor();
	
	UPROPERTY()
	AJCDigitalTwinPawn* OwnerPawn = nullptr;
	float SpringArmLengthTemp = 300.0;
};
