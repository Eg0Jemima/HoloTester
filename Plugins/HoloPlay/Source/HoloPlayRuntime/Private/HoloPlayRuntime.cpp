// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "HoloPlayRuntime.h"

#include "HoloPlaySettings.h"

#include "Render/SHoloPlayViewport.h"

#include "Render/HoloPlayViewportClient.h"

#include "Game/HoloPlayCapture.h"

#include "Misc/HoloPlayHelpers.h"
#include "Misc/HoloPlayLog.h"

#include "Managers/HoloPlayDisplayManager.h"
#include "Managers/HoloPlayCommandLineManager.h"
#include "Managers/HoloPlayLaunchManager.h"
#include "Managers/HoloPlayScalabilityManager.h"

#include "SceneViewport.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SlateApplication.h"
#include "ShaderCore.h"
#include "Engine.h"

#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#if WITH_EDITOR
#include "UnrealEd/Classes/Settings/LevelEditorPlaySettings.h"
#endif

#define LOCTEXT_NAMESPACE "FHoloPlayRuntimeModule"

IHoloPlayRuntime* GHoloPlayRuntime = nullptr;

FHoloPlayRuntimeModule::FHoloPlayRuntimeModule()
{
	GHoloPlayRuntime = this;
}

FHoloPlayRuntimeModule::~FHoloPlayRuntimeModule()
{
	GHoloPlayRuntime = nullptr;
}

void FHoloPlayRuntimeModule::StartupModule()
{
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/HoloPlay"), FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Plugins"), TEXT("HoloPlay"), TEXT("Shaders")));

	// Create all managers
	Managers.Add(HoloPlayLaunchManager = MakeShareable(new FHoloPlayLaunchManager()));
	Managers.Add(HoloPlayScalabilityManager = MakeShareable(new FHoloPlayScalabilityManager()));
	Managers.Add(HoloPlayCommandLineManager = MakeShareable(new FHoloPlayCommandLineManager()));
	Managers.Add(HoloPlayDisplayManager = MakeShareable(new FHoloPlayDisplayManager()));

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FHoloPlayRuntimeModule::OnPostEngineInit);
	UGameViewportClient::OnViewportCreated().AddRaw(this, &FHoloPlayRuntimeModule::OnGameViewportCreated);

	HoloPlayLoader.LoadDLL();
}

void FHoloPlayRuntimeModule::ShutdownModule()
{
	HoloPlayLoader.ReleaseDLL();

	// Release all managers
	for (auto Mng : Managers)
	{
		Mng->Release();
		Mng.Reset();
	}
	Managers.Empty();
}

void FHoloPlayRuntimeModule::StartPlayer(EHoloPlayModeType HoloPlayModeType)
{
	DISPLAY_HOLOPLAY_FUNC_TRACE(HoloPlayLogPlayer);

	if (bIsPlaying == false)
	{
		// Set Current HoloPlayModeType
		CurrentHoloPlayModeType = HoloPlayModeType;
		auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();
		bLockInMainViewport = HoloPlaySettings->HoloPlayWindowSettings.bLockInMainViewport;

		// Start all managers
		{
			bool Result = true;
			auto it = Managers.CreateIterator();
			while(Result && it)
			{
				Result = Result && (*it)->OnStartPlayer(CurrentHoloPlayModeType);
				++it;
			}

			if (!Result)
			{
				UE_LOG(HoloPlayLogPlayer, Verbose, TEXT("Error during StartPlayer managers"));
			}
		}

		switch (HoloPlayModeType)
		{
		case EHoloPlayModeType::PlayMode_InSeparateWindow:
		{
			bIsDestroyWindowRequested = false;

			const bool bUseBorderlessWindow = HoloPlaySettings->HoloPlayWindowSettings.bUseBorderlessWindow;
			EHoloPlayWindowType WindowType = HoloPlaySettings->HoloPlayWindowSettings.WindowType;
			EAutoCenter AutoCenter = static_cast<EAutoCenter>(HoloPlaySettings->HoloPlayWindowSettings.WindowAutoCenter);

			static FWindowStyle BorderlessStyle = FWindowStyle::GetDefault();
			BorderlessStyle
				.SetActiveTitleBrush(FSlateNoResource())
				.SetInactiveTitleBrush(FSlateNoResource())
				.SetFlashTitleBrush(FSlateNoResource())
				.SetOutlineBrush(FSlateNoResource())
				.SetBorderBrush(FSlateNoResource())
				.SetBackgroundBrush(FSlateNoResource())
				.SetChildBackgroundBrush(FSlateNoResource());

			FVector2D ClientSize;
			FVector2D ScreenPosition;
			auto HoloPlayDisplayMetrics = GetHoloPlayDisplayManager()->GetHoloPlayDisplayMetrics();
			auto DisplaySettings = GetHoloPlayDisplayManager()->GetDisplaySettings();
			auto CalibrationSettings = GetHoloPlayDisplayManager()->GetCalibrationSettings();
			if (HoloPlaySettings->HoloPlayWindowSettings.bAutoPlacementInHoloPlay)
			{
				ClientSize.X = CalibrationSettings.ScreenWidth;
				ClientSize.Y = CalibrationSettings.ScreenHeight;

				ScreenPosition.X = DisplaySettings.LKGxpos;
				ScreenPosition.Y = DisplaySettings.LKGypos;

				AutoCenter = EAutoCenter::None;
			}
			else
			{
				ClientSize = HoloPlaySettings->HoloPlayWindowSettings.ClientSize;
				ScreenPosition = HoloPlaySettings->HoloPlayWindowSettings.ScreenPosition;
			}

			HoloPlayWindow = SNew(SWindow)
				.Type(EWindowType::GameWindow)
				.Style(bUseBorderlessWindow ? &BorderlessStyle : &FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window"))
				.ClientSize(ClientSize)
				.AdjustInitialSizeAndPositionForDPIScale(false)
				.Title(FText::FromString("HoloPlay window"))
				.FocusWhenFirstShown(true)
				.ScreenPosition(ScreenPosition)
				.UseOSWindowBorder(HoloPlaySettings->HoloPlayWindowSettings.bUseOSWindowBorder)
				.CreateTitleBar(!bUseBorderlessWindow)
				.LayoutBorder(bUseBorderlessWindow ? FMargin(0) : FMargin(5, 5, 5, 5))
				.AutoCenter(AutoCenter)
				.SaneWindowPlacement(AutoCenter == EAutoCenter::None)
				.SizingRule(ESizingRule::UserSized);

			// 1. Add new window
			const bool bShowImmediately = false;
			FSlateApplication::Get().AddWindow(HoloPlayWindow.ToSharedRef(), bShowImmediately);

			// 2. Set window mode
			EWindowMode::Type WinMode = static_cast<EWindowMode::Type>(WindowType);
			// Do not set fullscreen mode here
			// The window mode will be set properly later
			if (WinMode == EWindowMode::Fullscreen)
			{
				HoloPlayWindow->SetWindowMode(EWindowMode::WindowedFullscreen);
			}
			else
			{
				HoloPlayWindow->SetWindowMode(WinMode);
			}

			// 3. Show window
			// No need to show window in off-screen rendering mode as it does not render to screen
			if (FSlateApplication::Get().IsRenderingOffScreen())
			{
				FSlateApplicationBase::Get().GetRenderer()->CreateViewport(HoloPlayWindow.ToSharedRef());
			}
			else
			{
				HoloPlayWindow->ShowWindow();
			}

			// 4. Tick now to force a redraw of the window and ensure correct fullscreen application
			FSlateApplication::Get().Tick();

			// 5. Add viewport to window
			// Create and assign viewport for Window. It could be with rendering directly to separate window or not if UE4 window, not native OS
			bool RenderDirectlyToWindowInSeparateWindow = false;
			if (!HoloPlaySettings->HoloPlayWindowSettings.bUseOSWindowBorder && !bUseBorderlessWindow)
			{
				RenderDirectlyToWindowInSeparateWindow = false;
			}
			else
			{
				RenderDirectlyToWindowInSeparateWindow = HoloPlaySettings->HoloPlayWindowSettings.RenderDirectlyToWindowInSeparateWindow;
			}
			HoloPlayViewport = SNew(SHoloPlayViewport).RenderDirectlyToWindow(RenderDirectlyToWindowInSeparateWindow);
			HoloPlayViewport->GetHoloPlayViewportClient()->SetViewportWindow(HoloPlayWindow);
			HoloPlayWindow->SetContent(HoloPlayViewport.ToSharedRef());
			HoloPlayWindow->SlatePrepass();
			if (WinMode != HoloPlayWindow->GetWindowMode())
			{
				HoloPlayWindow->SetWindowMode(WinMode);
				HoloPlayWindow->ReshapeWindow(ScreenPosition, ClientSize);
				FVector2D NewViewportSize = HoloPlayWindow->GetViewportSize();
				HoloPlayViewport->GetSceneViewport()->UpdateViewportRHI(false, NewViewportSize.X, NewViewportSize.Y, WinMode, PF_Unknown);
				HoloPlayViewport->GetSceneViewport()->Invalidate();

				// Resize backbuffer
				FVector2D NewBackbufferSize = HoloPlayWindow->IsMirrorWindow() ? ClientSize : NewViewportSize;
				FSlateApplicationBase::Get().GetRenderer()->UpdateFullscreenState(HoloPlayWindow.ToSharedRef(), NewBackbufferSize.X, NewBackbufferSize.Y);
			}

			// 6. Add window delegates
			OnWindowClosedDelegate.BindRaw(this, &FHoloPlayRuntimeModule::OnWindowClosed);
			HoloPlayWindow->SetOnWindowClosed(OnWindowClosedDelegate);

			break;
		}
		case EHoloPlayModeType::PlayMode_InMainViewport:
		{
			if (GEngine->GameViewport)
			{
				bool bRenderDirectlyToWindow = (GIsEditor) ? false : true;

				TSharedPtr<SWindow> GameViewportWindow = GEngine->GameViewport->GetWindow();
				FString WindowTitle = GameViewportWindow->GetTitle().ToString();
				UMovieSceneCapture* MovieSceneCapture = HoloPlay::GetMovieSceneCapture();

				FVector2D ClientSize;
				FVector2D ScreenPosition;
				FMonitorInfo FirstHoloPlayDisplayInfo;
				auto HoloPlayDisplayMetrics = GetHoloPlayDisplayManager()->GetHoloPlayDisplayMetrics();
				auto DisplaySettings = GetHoloPlayDisplayManager()->GetDisplaySettings();
				auto CalibrationSettings = GetHoloPlayDisplayManager()->GetCalibrationSettings();
				if (HoloPlaySettings->HoloPlayWindowSettings.bAutoPlacementInHoloPlay)
				{
					ClientSize.X = CalibrationSettings.ScreenWidth;
					ClientSize.Y = CalibrationSettings.ScreenHeight;

					ScreenPosition.X = DisplaySettings.LKGxpos;
					ScreenPosition.Y = DisplaySettings.LKGypos;

					bool bRequesWindowAutoPlacement = false;
#if WITH_EDITOR
					auto LastExecutedPlayModeType = GetDefault<ULevelEditorPlaySettings>()->LastExecutedPlayModeType;

					// If Movie capturing
					if ( (MovieSceneCapture != nullptr && MovieSceneCapture->IsRooted()) || bIsCaptureStandaloneMovie)
					{
						bRenderDirectlyToWindow = false;
					}
					//  if Editor Floating window
					else if (LastExecutedPlayModeType == EPlayModeType::PlayMode_InEditorFloating)
					{
						FMargin Border = GameViewportWindow->GetWindowBorderSize();
						ScreenPosition.X -= Border.Left;
						ClientSize.X += Border.Left + Border.Right;
						ScreenPosition.Y -= Border.Top;
						ClientSize.Y += Border.Top + Border.Bottom;

						GameViewportWindow->ReshapeWindow(ScreenPosition, ClientSize);
					}
					// Check placement if bIsStandaloneGame or bIsGameMode
					else if (bIsStandaloneGame || bIsGameMode)
					{

						FSystemResolution::RequestResolutionChange(ClientSize.X, ClientSize.Y, GameViewportWindow->GetWindowMode());
						GameViewportWindow->ReshapeWindow(ScreenPosition, ClientSize);
					}
#else
					{
						FSystemResolution::RequestResolutionChange(ClientSize.X, ClientSize.Y, GameViewportWindow->GetWindowMode());
						GameViewportWindow->ReshapeWindow(ScreenPosition, ClientSize);
					}
#endif
				}
				GEngine->GameViewport->OnCloseRequested().AddRaw(this, &FHoloPlayRuntimeModule::OnGameViewportCloseRequested);

				// Create and assign viewport to window
				HoloPlayViewport = SNew(SHoloPlayViewport).RenderDirectlyToWindow(bRenderDirectlyToWindow);
				HoloPlayViewport->GetHoloPlayViewportClient()->SetViewportWindow(GEngine->GameViewport->GetWindow());

				GEngine->GameViewport->bDisableWorldRendering = true;
				GEngine->GameViewport->AddViewportWidgetContent(
					HoloPlayViewport.ToSharedRef()
				);
			}
			break;
		}
		default:
			return;
		}

		bIsPlaying = true;
	}
}

void FHoloPlayRuntimeModule::StopPlayer()
{
	DISPLAY_HOLOPLAY_FUNC_TRACE(HoloPlayLogPlayer);

	if (bIsPlaying == true)
	{
		// Stop all managers
		for (auto Mng : Managers)
		{
			Mng->OnStopPlayer();
		}

		auto PlayModeType = CurrentHoloPlayModeType;

		// Stop PlayInStandalon first
		if (bIsStandaloneGame || bIsCaptureStandaloneMovie || bLockInMainViewport)
		{
			PlayModeType = EHoloPlayModeType::PlayMode_InMainViewport;
		}

		switch (PlayModeType)
		{
		case EHoloPlayModeType::PlayMode_InSeparateWindow:
		{
			if (bIsDestroyWindowRequested == false)
			{
				bIsDestroyWindowRequested = true;

				if (FSlateApplication::IsInitialized())
				{
					HoloPlayWindow->RequestDestroyWindow();
				}
				else
				{
					HoloPlayWindow->DestroyWindowImmediately();
				}

				// Stop player
				OnWindowClosedDelegate.Unbind();

				HoloPlayViewport.Reset();
				HoloPlayWindow.Reset();
			}

			break;
		}
		case EHoloPlayModeType::PlayMode_InMainViewport:
		{
			if (GEngine->GameViewport)
			{
				GEngine->GameViewport->bDisableWorldRendering = false;
				GEngine->GameViewport->RemoveViewportWidgetContent(
					HoloPlayViewport.ToSharedRef()
				);

				HoloPlayViewport.Reset();
			}

			break;
		}
		default:
			return;
		}

		bIsPlaying = false;
	}
}

void FHoloPlayRuntimeModule::RestartPlayer(EHoloPlayModeType HoloPlayModeType)
{
	StopPlayer();
	StartPlayer(HoloPlayModeType);
}

void FHoloPlayRuntimeModule::OnWindowClosed(const TSharedRef<SWindow>& Window)
{
	StopPlayer();
}

void FHoloPlayRuntimeModule::OnGameViewportCloseRequested(FViewport* InViewport)
{
	StopPlayer();
}

TSharedPtr<FHoloPlayDisplayManager> FHoloPlayRuntimeModule::GetHoloPlayDisplayManager()
{
	return HoloPlayDisplayManager;
}

TSharedPtr<FHoloPlayCommandLineManager> FHoloPlayRuntimeModule::GetHoloPlayCommandLineManager()
{
	return HoloPlayCommandLineManager;
}

TSharedPtr<FHoloPlayScalabilityManager> FHoloPlayRuntimeModule::GetHoloPlayScalabilityManager()
{
	return HoloPlayScalabilityManager;
}

void FHoloPlayRuntimeModule::StartPlayerSeparateProccess()
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();
	bLockInMainViewport = HoloPlaySettings->HoloPlayWindowSettings.bLockInMainViewport;
	auto LastExecutedPlayModeType = bLockInMainViewport ? EHoloPlayModeType::PlayMode_InMainViewport : HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType;

#if WITH_EDITOR
	// Try to parse if this is standalone mode
	{
		FString StandaloneSessionName = "Play in Standalone Game";
		bIsStandaloneGame = FApp::GetSessionName() == StandaloneSessionName;
		if (bIsStandaloneGame)
		{
			StartPlayer(EHoloPlayModeType::PlayMode_InMainViewport);
			return;
		}
	}

	// Try to parse separate process in capture scene
	{
		FString CaptureStandaloneMovie;
		// Try to read screen model from command line
		FParse::Value(FCommandLine::Get(), TEXT("-MovieSceneCaptureManifest="), CaptureStandaloneMovie);

		bIsCaptureStandaloneMovie = !CaptureStandaloneMovie.IsEmpty();
		if (bIsCaptureStandaloneMovie)
		{
			StartPlayer(EHoloPlayModeType::PlayMode_InMainViewport);
			return;
		}
	}

	// Play in editor game mode
	{
		
		bool OnOff = true;
		bIsGameMode = FParse::Bool(FCommandLine::Get(), TEXT("-game"), OnOff);
		if (bIsGameMode)
		{
			StartPlayer(LastExecutedPlayModeType);
			return;
		}
	}

#else
	StartPlayer(LastExecutedPlayModeType);
#endif
}

void FHoloPlayRuntimeModule::OnPostEngineInit()
{
	// Init all managers
	InitAllManagers();

	// Init all settings
	GetDefault<UHoloPlaySettings>()->PostEngineInit();
}

void FHoloPlayRuntimeModule::OnGameViewportCreated()
{
	if (!GIsEditor)
	{
		// We're in game mode
		if (ensureMsgf(bSeparateProccessPlayerStarded == false, TEXT("StartPlayer in separate proccess was already called")))
		{
			// Init all managers
			InitAllManagers();

			StartPlayerSeparateProccess();
			bSeparateProccessPlayerStarded = true;
		}
	}
}

void FHoloPlayRuntimeModule::InitAllManagers()
{
	if (bIsManagerInit == false)
	{
		bIsManagerInit = true;

		bool Result = true;
		auto it = Managers.CreateIterator();
		while (Result && it)
		{
			Result = Result && (*it)->Init();
			++it;
		}

		if (!Result)
		{
			UE_LOG(HoloPlayLogPlayer, Verbose, TEXT("Error during initialize managers"));
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHoloPlayRuntimeModule, HoloPlayRuntime)