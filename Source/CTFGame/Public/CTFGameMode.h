// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TeamColors.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CTFGAME_API ACTFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACTFGameMode();

	void FlagCaptured(ETeamColor ScoringTeam);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;




};
