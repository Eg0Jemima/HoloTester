// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HoloPlaySettings.h"
#include "HoloPlayProSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class HOLOPLAYPRORUNTIME_API UHoloPlayProSettings : public UObject
{
	GENERATED_BODY()
public:
	UHoloPlayProSettings();

	UPROPERTY(GlobalConfig, EditAnywhere, Category = HoloPlayProSetting)
	uint32 bIsMostRightAutoPlacement : 1;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = HoloPlayProSetting)
	uint32 bIsEnabledInStandalone : 1;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = HoloPlayProSetting)
	FVector2D ClientSize = FVector2D(800, 1200);

	UPROPERTY(GlobalConfig, EditAnywhere, Category = HoloPlayProSetting)
	FVector2D ScreenPosition = FVector2D(6400, 0);

	UPROPERTY(GlobalConfig, EditAnywhere, Category = HoloPlayProSetting)
	EHoloPlayWindowType WindowType = EHoloPlayWindowType::WindowedFullscreen;
};
