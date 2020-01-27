#include "HoloPlayProInput.h"
#include "HoloPlayProInputDevice.h"

#include "GameFramework/InputSettings.h"
#include "Engine.h"

FHoloPlayProInput::FHoloPlayProInput()
{
}

FHoloPlayProInput::~FHoloPlayProInput()
{
}

void FHoloPlayProInput::StartupModule()
{
	IInputDeviceModule::StartupModule();

	FHoloPlayProInputDevice::PreInit();

	// Enable gestures by default
	GetMutableDefault<UInputSettings>()->bEnableGestureRecognizer = true;
}

void FHoloPlayProInput::ShutdownModule()
{
}

TSharedPtr< class IInputDevice > FHoloPlayProInput::CreateInputDevice( const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler )
{
	TSharedPtr<FHoloPlayProInputDevice> HoloPlayProInputDevice(new FHoloPlayProInputDevice(InMessageHandler));

	return HoloPlayProInputDevice;
}

IMPLEMENT_MODULE( FHoloPlayProInput, HoloPlayProInput)
