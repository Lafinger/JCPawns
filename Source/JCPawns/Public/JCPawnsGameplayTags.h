#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace JCPawnsGameplayTags
{
	JCPAWNS_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);
	
	JCPAWNS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(JC_Input_GroundMove);
	JCPAWNS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(JC_Input_ViewMove);
	JCPAWNS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(JC_Input_Look);
	JCPAWNS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(JC_Input_Zoom);
};
