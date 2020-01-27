/*
*
* Copyright (C) <2014> samiljan <Sam Persson>, tsky <thomas.kollakowksy@w-hs.de>
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license.  See the LICENSE file for details.
*/

#include "JoystickPlugin.h"
#include <Engine.h>

#if WITH_EDITOR
	#include "InputSettingsCustomization.h"
#endif

IMPLEMENT_MODULE(FJoystickPlugin, JoystickPlugin)

#define LOCTEXT_NAMESPACE "JoystickPlugin"

#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

void FJoystickPlugin::StartupModule()
{
	SDLLoader.LoadDLL();

	IJoystickPlugin::StartupModule();
	JoystickDevice = MakeShareable(new ::FJoystickDevice());

#if WITH_EDITOR
	// Replace parts of the input settings widget to make them wide enough to fit long joystick names
	FInputActionMappingCustomizationExtended::Register();
	FInputAxisMappingCustomizationExtended::Register();
#endif
}

void FJoystickPlugin::ShutdownModule()
{
	IJoystickPlugin::ShutdownModule();
	JoystickDevice = nullptr;

	SDLLoader.ReleaseDLL();
}

#undef LOCTEXT_NAMESPACE
