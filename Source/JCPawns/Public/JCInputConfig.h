// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "JCInputConfig.generated.h"

class UInputAction;

/**
 * FJCInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FJCTaggedInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction;
};

/**
 * UJCInputConfig
 *
 *	Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class JCPAWNS_API UJCInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	const UInputAction* FindBaseInputActionWithTag(const FGameplayTag& InputTag) const
	{
		if(TaggedBaseInputActions.IsEmpty())
		{
			ensureAlwaysMsgf(false, TEXT("Base input actions are empty!"));
			return nullptr;
		}
		
		for (const FJCTaggedInputAction& TaggedInputAction : TaggedBaseInputActions)
		{
			if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
			{
				return TaggedInputAction.InputAction;
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Cannot find input action by tag!"));
		return nullptr;
	}
	
	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FJCTaggedInputAction> TaggedBaseInputActions;
};
