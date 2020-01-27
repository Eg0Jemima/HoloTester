// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IHoloPlayProInput.h"
#include "IInputDevice.h"

class FHoloPlayProInputMessageHandler;

class FHoloPlayProInput : public IHoloPlayProInput
{
public:
	FHoloPlayProInput();
	virtual ~FHoloPlayProInput();

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	/** IInputDeviceModule interface*/
	virtual TSharedPtr< class IInputDevice > CreateInputDevice( const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler ) override;
};
