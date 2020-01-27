#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"

#include "IInputDevice.h"
#include "GenericPlatform/IInputInterface.h"
#include "Windows/WindowsApplication.h"

class FHoloPlayProInputDevice : public IInputDevice, IWindowsMessageHandler
{
public:
	// WindowsMixedRealitySpatialInput.cpp
	FHoloPlayProInputDevice(const TSharedRef< FGenericApplicationMessageHandler > & InMessageHandler);
	virtual ~FHoloPlayProInputDevice();
	static void PreInit();

	// Inherited via IInputDevice
	virtual void Tick(float DeltaTime) override {}
	virtual void SendControllerEvents() override {}
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool Exec(UWorld * InWorld, const TCHAR * Cmd, FOutputDevice & Ar) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues & values) override {}

	// Begin IWindowsMessageHandler interface
	virtual bool ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult) override;
	// End IWindowsMessageHandler interface

private:
	void OnHoloPlayInputKey(FViewport * InViewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad);

private:
	FDelegateHandle OnHoloPlayInputDelegateHandle;
	double LastClickPressTime = 0;

	// Unreal message handler.
	TSharedPtr< FGenericApplicationMessageHandler > MessageHandler;
};