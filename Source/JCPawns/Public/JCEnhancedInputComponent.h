// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "JCInputConfig.h"

#include "JCEnhancedInputComponent.generated.h"

// class UObject;


/**
 * ULyraInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class UJCEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	template<class UserClass, typename FuncType>
	void BindJCInputAction(const UJCInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
	{
		ensureAlwaysMsgf(InputConfig, TEXT("Input config is empty!"));
		if (const UInputAction* IA = InputConfig->FindBaseInputActionWithTag(InputTag))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}
	}
};
