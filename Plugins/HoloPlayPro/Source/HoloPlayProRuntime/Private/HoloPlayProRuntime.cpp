#include "HoloPlayProRuntime.h"
#include "Slate/SHoloPlayProViewport.h"
#include "HoloPlayProSettings.h"

#include "IHoloPlayRuntime.h"
#include "Managers/HoloPlayDisplayManager.h"

#include "Framework/Application/SlateApplication.h"

IHoloPlayProRuntime* GHoloPlayProRuntime = nullptr;

FHoloPlayProRuntime::FHoloPlayProRuntime()
	: bIsExtendedScreenRunning(false)
{
	GHoloPlayProRuntime = this;
}

FHoloPlayProRuntime::~FHoloPlayProRuntime()
{
	GHoloPlayProRuntime = nullptr;
}

void FHoloPlayProRuntime::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FHoloPlayProRuntime::OnPostEngineInit);
}

void FHoloPlayProRuntime::ShutdownModule()
{

}

void FHoloPlayProRuntime::RunExtendedScreen()
{
	if (IsExtendedScreenRunning())
	{
		return;
	}

	auto HoloPlayProSettings = GetDefault<UHoloPlayProSettings>();
	FVector2D ScreenPosition = HoloPlayProSettings->ScreenPosition;
	FVector2D ClientSize = HoloPlayProSettings->ClientSize;

	// If auto placement
	if (HoloPlayProSettings->bIsMostRightAutoPlacement)
	{
		auto HoloPlayDisplayManager = IHoloPlayRuntime::Get().GetHoloPlayDisplayManager();
		auto DisplayMetrics = HoloPlayDisplayManager->GetDisplayMetrics();

		FMonitorInfo MostLeftMonitorInfo;
		if (DisplayMetrics.MonitorInfo.Num())
		{
			MostLeftMonitorInfo = DisplayMetrics.MonitorInfo[0];
		}
	
		if (HoloPlayDisplayManager.IsValid())
		{
			for (FMonitorInfo MonitorInfo : DisplayMetrics.MonitorInfo)
			{
				if (MonitorInfo.DisplayRect.Left > MostLeftMonitorInfo.DisplayRect.Left)
				{
					MostLeftMonitorInfo = MonitorInfo;
				}
			}
		}

		ScreenPosition.X = MostLeftMonitorInfo.DisplayRect.Left;
		ScreenPosition.Y = MostLeftMonitorInfo.DisplayRect.Top;

		ClientSize.X = MostLeftMonitorInfo.DisplayRect.Right - MostLeftMonitorInfo.DisplayRect.Left;
		ClientSize.Y = MostLeftMonitorInfo.DisplayRect.Bottom - MostLeftMonitorInfo.DisplayRect.Top;
	}

	bool bUseBorderlessWindow = true;
	const FText WindowTitle = FText::FromString("ExtendedScreenWindow");
	EAutoCenter AutoCenterType = EAutoCenter::None;
	bool bShouldPreserveAspectRatio = true;
	bool bAllowWindowResize = true;
	const bool bAllowClose = true;
	const bool bAllowMaximize = true;
	const bool bAllowMinimize = true;

	EWindowMode::Type WindowMode = EWindowMode::ConvertIntToWindowMode((int)HoloPlayProSettings->WindowType);

	static FWindowStyle BorderlessStyle = FWindowStyle::GetDefault();
	BorderlessStyle
		.SetActiveTitleBrush(FSlateNoResource())
		.SetInactiveTitleBrush(FSlateNoResource())
		.SetFlashTitleBrush(FSlateNoResource())
		.SetOutlineBrush(FSlateNoResource())
		.SetBorderBrush(FSlateNoResource())
		.SetBackgroundBrush(FSlateNoResource())
		.SetChildBackgroundBrush(FSlateNoResource());

	ExtendedScreenWindow = SNew(SWindow)
		.Type(EWindowType::GameWindow)
		.Style(bUseBorderlessWindow ? &BorderlessStyle : &FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window"))
		.ClientSize(ClientSize)
		.AdjustInitialSizeAndPositionForDPIScale(false)
		.Title(WindowTitle)
		.AutoCenter(AutoCenterType)
		.ScreenPosition(ScreenPosition)
		.MinWidth(100)
		.MinHeight(100)
		.MaxWidth(10000)
		.MaxHeight(10000)
		.FocusWhenFirstShown(true)
		.SaneWindowPlacement(AutoCenterType == EAutoCenter::None)
		.UseOSWindowBorder(!bUseBorderlessWindow)
		.CreateTitleBar(!bUseBorderlessWindow)
		.ShouldPreserveAspectRatio(bShouldPreserveAspectRatio)
		.LayoutBorder(bUseBorderlessWindow ? FMargin(0) : FMargin(5, 5, 5, 5))
		.SizingRule(bAllowWindowResize ? ESizingRule::UserSized : ESizingRule::FixedSize)
		.HasCloseButton(bAllowClose)
		.SupportsMinimize(bAllowMinimize)
		.SupportsMaximize(bAllowMaximize);

	const bool bShowImmediately = false;

	FSlateApplication::Get().AddWindow(ExtendedScreenWindow.ToSharedRef(), bShowImmediately);

	// Do not set fullscreen mode here, since it doesn't take 
	// HMDDevice into account. The window mode will be set properly later
	// from SwitchGameWindowToUseGameViewport() method (see ResizeWindow call).
	if (WindowMode == EWindowMode::Fullscreen)
	{
		ExtendedScreenWindow->SetWindowMode(EWindowMode::WindowedFullscreen);
	}
	else
	{
		ExtendedScreenWindow->SetWindowMode(WindowMode);
	}

	// No need to show window in off-screen rendering mode as it does not render to screen
	if (FSlateApplication::Get().IsRenderingOffScreen())
	{
		FSlateApplicationBase::Get().GetRenderer()->CreateViewport(ExtendedScreenWindow.ToSharedRef());
	}
	else
	{
		ExtendedScreenWindow->ShowWindow();
	}

	// Tick now to force a redraw of the window and ensure correct fullscreen application
	FSlateApplication::Get().Tick();

	bIsExtendedScreenRunning = true;

	// Create Viewport
	HoloPlayProViewport = SNew(SHoloPlayProViewport);

	ExtendedScreenWindow->SetContent(HoloPlayProViewport.ToSharedRef());
	ExtendedScreenWindow->SlatePrepass();

	// Add window delegates
	OnWindowClosedDelegate.BindRaw(this, &FHoloPlayProRuntime::OnWindowClosed);
	ExtendedScreenWindow->SetOnWindowClosed(OnWindowClosedDelegate);
}

void FHoloPlayProRuntime::StopExtendedScreen(bool bCloseWindow)
{
	if (!IsExtendedScreenRunning())
	{
		return;
	}

	if (bCloseWindow)
	{
		if (GIsEditor)
		{
			ExtendedScreenWindow->RequestDestroyWindow();
		}
		else
		{
			ExtendedScreenWindow->DestroyWindowImmediately();
		}
	}

	if (HoloPlayProViewport.IsValid())
	{
		HoloPlayProViewport->Release();
		HoloPlayProViewport.Reset();
		HoloPlayProViewport = nullptr;
	}

	bIsExtendedScreenRunning = false;

	OnWindowClosedDelegate.Unbind();
}

bool FHoloPlayProRuntime::IsExtendedScreenRunning()
{
	return bIsExtendedScreenRunning;
}

UHoloPlayProViewportClient * FHoloPlayProRuntime::GetHoloPlayProViewportClient()
{
	if (HoloPlayProViewport.IsValid())
	{
		return HoloPlayProViewport->GetHoloPlayProViewportClient();
	}

	return nullptr;
}

TWeakPtr<SHoloPlayProViewport> FHoloPlayProRuntime::GetSHoloPlayProViewport()
{
	return HoloPlayProViewport;
}

void FHoloPlayProRuntime::OnWindowClosed(const TSharedRef<SWindow>& Window)
{
	StopExtendedScreen(false);
}

void FHoloPlayProRuntime::OnPostEngineInit()
{
	auto HoloPlayProSettings = GetDefault<UHoloPlayProSettings>();

	if (!GIsEditor && HoloPlayProSettings->bIsEnabledInStandalone)
	{
		RunExtendedScreen();
	}
}


IMPLEMENT_MODULE( FHoloPlayProRuntime, HoloPlayProRuntime)
