#pragma once

#include "CoreMinimal.h"
#include "TeamColors.generated.h"

UENUM(BlueprintType)
enum class ETeamColor : uint8
{
    Red     UMETA(DisplayName = "Red"),
    Blue    UMETA(DisplayName = "Blue"),
    Yellow  UMETA(DisplayName = "Yellow"),
    Green   UMETA(DisplayName = "Green"),
    None    UMETA(DisplayName = "None")
};
