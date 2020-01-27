#include "HoloPlayProInputDevice.h"
#include "InputCoreTypes.h"
#include "HoloPlayProInputDefinitions.h"

#include "IHoloPlayRuntime.h"

#include "GameFramework/InputSettings.h"
#include "Engine.h"
#include "Framework/Application/SlateApplication.h"


#define HoloPlayProInputCategoryName "HoloPlayPro"

#define LOCTEXT_NAMESPACE "HoloPlayProInputDevice"

void FHoloPlayProInputDevice::PreInit()
{
	EKeys::AddKey(FKeyDetails(
		FHoloPlayProInputKeys::GlobalGesture_DoubleTap,
		LOCTEXT(GlobalGestureDoubleTapName, GlobalGestureDoubleTapFriendlyName),
		0,
		HoloPlayProInputCategoryName));

	EKeys::AddKey(FKeyDetails(
		FHoloPlayProInputKeys::HoloPlayViewportGesture_DoubleTap,
		LOCTEXT(HoloPlayViewportGestureDoubleTapName, HoloPlayViewportGestureDoubleTapFriendlyName),
		0,
		HoloPlayProInputCategoryName));
}

FHoloPlayProInputDevice::FHoloPlayProInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
	FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
	check(WindowsApplication);

	WindowsApplication->AddMessageHandler(*this);


	OnHoloPlayInputDelegateHandle = IHoloPlayRuntime::Get().OnHoloPlayInputKeyDelegate().AddRaw(this, &FHoloPlayProInputDevice::OnHoloPlayInputKey);
}

FHoloPlayProInputDevice::~FHoloPlayProInputDevice()
{
	IHoloPlayRuntime::Get().OnHoloPlayInputKeyDelegate().Remove(OnHoloPlayInputDelegateHandle);
}

void FHoloPlayProInputDevice::OnHoloPlayInputKey(FViewport * InViewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	if (Key.IsMouseButton() && EInputEvent::IE_Pressed == EventType)
	{
		double TimeNow = FPlatformTime::Seconds();
		double Delta = TimeNow - LastClickPressTime;

		if (Delta < GetDefault<UInputSettings>()->DoubleClickTime)
		{
			MessageHandler->OnControllerButtonPressed(
				FHoloPlayProInputKeys::HoloPlayViewportGesture_DoubleTap.GetFName(),
				static_cast<int32>(0),
				false);
		}

		LastClickPressTime = TimeNow;
	}
}

void FHoloPlayProInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FHoloPlayProInputDevice::Exec(UWorld * InWorld, const TCHAR * Cmd, FOutputDevice & Ar)
{
	return false;
}

bool FHoloPlayProInputDevice::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32 & OutResult)
{
	bool bHandled = false;
	switch (msg)
	{
		// Mouse Button Down
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
		{
			EMouseButtons::Type MouseButton = EMouseButtons::Invalid;
			bool bDoubleClick = false;
			bool bMouseUp = false;
			switch (msg)
			{
			case WM_LBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Left;
				break;
			case WM_LBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Left;
				break;
			case WM_LBUTTONDOWN:
				MouseButton = EMouseButtons::Left;
				break;
			case WM_MBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_MBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_MBUTTONDOWN:
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_RBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Right;
				break;
			case WM_RBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Right;
				break;
			case WM_RBUTTONDOWN:
				MouseButton = EMouseButtons::Right;
				break;
			case WM_XBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			case WM_XBUTTONUP:
				bMouseUp = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			case WM_XBUTTONDOWN:
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			default:
				check(0);
			}

			if (bMouseUp)
			{
			}
			else if (bDoubleClick)
			{
				MessageHandler->OnControllerButtonPressed(
					FHoloPlayProInputKeys::GlobalGesture_DoubleTap.GetFName(),
					static_cast<int32>(0),
					false);
			}
			else
			{
			}
			return 0;
		}
		break;
	}

	return bHandled;
}

#undef LOCTEXT_NAMESPACE