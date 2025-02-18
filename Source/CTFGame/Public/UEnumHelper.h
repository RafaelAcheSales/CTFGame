#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TeamColors.h"
#include "UEnumHelper.generated.h"

UCLASS()
class CTFGAME_API UEnumHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Enum")
    static FString EnumToString(ETeamColor TeamColor)
    {
        switch (TeamColor)
        {
        case ETeamColor::Red:    return TEXT("Red");
        case ETeamColor::Blue:   return TEXT("Blue");
        case ETeamColor::Yellow: return TEXT("Yellow");
        case ETeamColor::Green:  return TEXT("Green");
        default: return TEXT("Unknown");
        }
    }
};
