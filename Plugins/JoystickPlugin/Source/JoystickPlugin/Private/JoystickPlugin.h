#pragma once

#include <InputDevice.h>

#include "IJoystickPlugin.h"
#include "JoystickDevice.h"

#include "ISDLModule.h"

class FJoystickPlugin : public IJoystickPlugin
{
public:
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override
	{
		return JoystickDevice;
	}

	void ShutdownModule() override;

	void StartupModule() override;

	TSharedPtr< class FJoystickDevice > JoystickDevice;

private:
	FSDLLoader SDLLoader;
};
