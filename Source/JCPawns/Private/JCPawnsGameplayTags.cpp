#include "JCPawnsGameplayTags.h"

namespace JCPawnsGameplayTags
{
	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					// WHLog(FString::Printf(TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString()), EDC_Default, EDV_Warning);
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}

	// Gameplay Tag : Input
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JC_Input_GroundMove, "JC.Pawn.DigitalTwin.Move.Ground", "Move pawn base on the ground.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JC_Input_ViewMove, "JC.Pawn.DigitalTwin.Move.View", "Camera Rotate");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JC_Input_Look, "JC.Pawn.DigitalTwin.Look", "Camera Zoom");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JC_Input_Zoom, "JC.Pawn.DigitalTwin.Zoom", "Camera Sprint");
}
