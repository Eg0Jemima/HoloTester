#include "Render/HoloPlayViewportClient.h"
#include "Render/HoloPlayRendering.h"
#include "Game/HoloPlayCapture.h"
#include "Managers/HoloPlayScalabilityManager.h"
#include "Misc/HoloPlayLog.h"
#include "Misc/HoloPlayStats.h"
#include "IHoloPlayRuntime.h"

#include "CanvasTypes.h"
#include "ClearQuad.h"

#include "CanvasItem.h"
#include "Engine.h"
#include "ImageUtils.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Public/ScreenRendering.h"
#include "Engine/Console.h"

void FHoloPlayScreenshotRequest::RequestScreenshot()
{
	// empty string means we'll later pick the name
	RequestScreenshot(TEXT(""), true);
}

void FHoloPlayScreenshotRequest::RequestScreenshot(const FString & InFilename, bool bAddFilenameSuffix)
{
	FString GeneratedFilename = InFilename;
	CreateViewportScreenShotFilename(GeneratedFilename);

	if (bAddFilenameSuffix)
	{
		const bool bRemovePath = false;
		GeneratedFilename = FPaths::GetBaseFilename(GeneratedFilename, bRemovePath);
		FFileHelper::GenerateNextBitmapFilename(GeneratedFilename, TEXT("png"), Filename);
	}
	else
	{
		Filename = GeneratedFilename;
		if (FPaths::GetExtension(Filename).Len() == 0)
		{
			Filename += TEXT(".png");
		}
	}
}

void FHoloPlayScreenshotRequest::CreateViewportScreenShotFilename(FString & InOutFilename)
{
	FString TypeName;

	TypeName = InOutFilename.IsEmpty() ? TEXT("Screenshot") : InOutFilename;
	check(!TypeName.IsEmpty());

	//default to using the path that is given
	InOutFilename = TypeName;
	if (!TypeName.Contains(TEXT("/")))
	{
		InOutFilename = GetDefault<UEngine>()->GameScreenshotSaveDirectory.Path / TypeName;
	}
}

TArray<FColor>* FHoloPlayScreenshotRequest::GetHighresScreenshotMaskColorArray()
{
	return &HighresScreenshotMaskColorArray;
}


void FHoloPlayLenticularScreenshotRequest::RequestScreenshot(bool bInShowUI)
{
	// empty string means we'll later pick the name
	RequestScreenshot(TEXT(""), bInShowUI, true);
}

void FHoloPlayLenticularScreenshotRequest::RequestScreenshot(const FString & InFilename, bool bInShowUI, bool bAddFilenameSuffix)
{
	FHoloPlayScreenshotRequest::RequestScreenshot(InFilename, bAddFilenameSuffix);

	// Register the screenshot
	if (!Filename.IsEmpty())
	{
		bShowUI = bInShowUI;
	}
}

FHoloPlayViewportClient::FHoloPlayViewportClient()
	: bIgnoreInput(false)
	, CurrentMouseCursor(EMouseCursor::Default)
	, QuiltRT(nullptr)
	, Viewport(nullptr)
{
}

FHoloPlayViewportClient::~FHoloPlayViewportClient()
{
}

void FHoloPlayViewportClient::Draw(FViewport * InViewport, FCanvas * InCanvas)
{
	check(IsInGameThread());

	SCOPE_CYCLE_COUNTER(STAT_Draw_GameThread);

	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();
	auto RenderingSettings = HoloPlaySettings->HoloPlayRenderingSettings;
	auto HoloPlayCapture = GetGameHoloPlayCapture();

	// Clear entire canvas
	InCanvas->Clear(FLinearColor::Black);
	
	if (!HoloPlayCapture.IsValid())
	{
		UE_LOG(HoloPlayLogInput, Verbose, TEXT(">> HoloPlayCapture Not valid"));
		return;
	}

	if (!HoloPlayCapture->GetRenderCamera())
	{
		InCanvas->Clear(FLinearColor::Blue);
		return;
	}
	
	// create render QuiltRT if not exists
	GetQuiltRT(HoloPlayCapture);

	if (!HoloPlayCapture->GetRenderCamera()->TextureTarget)
	{
		ensureMsgf(false, TEXT("HoloPlayCapture->GetRenderCamera()->TextureTarget does not exist"));
		InCanvas->Clear(FLinearColor::Green);
		return;
	}

	bool bIsOverrideQuiltTexture2D = HoloPlayCapture->GetOverrideQuiltTexture2D() != nullptr;
	HoloPlay::FRenderContext RenderContext =
	{
		InViewport,
		QuiltRT->GameThread_GetRenderTargetResource(),
		HoloPlayCapture->GetTilingValues(),
		HoloPlayCapture->GetRenderCamera()->TextureTarget->Resource,
		bIsOverrideQuiltTexture2D ? HoloPlayCapture->GetOverrideQuiltTexture2D()->Resource : nullptr,
		DuplicateObject(GetDefault<UHoloPlaySettings>(), nullptr)
	};

	// Process Screenshot 2Ds before offset Tiling scene capture
	ProcessScreenshot2D(HoloPlayCapture);

	// Copy to quilt. Render only if no quilt override
	if (!bIsOverrideQuiltTexture2D)
	{
		int CurrentView = 0;
		int NumViews = HoloPlayCapture->GetTilingValues().NumViews;

		for (CurrentView; CurrentView < HoloPlayCapture->GetTilingValues().NumViews; CurrentView++)
		{
			if (RenderingSettings.bRenderScene)
			{
				HoloPlayCapture->RenderView(CurrentView, NumViews);
			}

			bool bCopyToQuilt = RenderingSettings.bCopyToQuilt;
			ENQUEUE_RENDER_COMMAND(CopyToQuiltCommand)(
				[&, RenderContext, CurrentView, bCopyToQuilt](FRHICommandListImmediate& RHICmdList)
			{
				SCOPE_CYCLE_COUNTER(STAT_CopyToQuiltShader_RenderThread);

				if (bCopyToQuilt)
				{
					HoloPlay::CopyToQuiltShader_RenderThread(RHICmdList, RenderContext, CurrentView);
				}
			});
		}
	}

	// Synchronize game and rendering thread before lenticular shader
	FlushRenderingCommands();

	// Process Quilt screenshot we should call. but ProcessScreenShots for Quilt Screenshots called in FViewport->Draw()
	ProcessScreenshotQuilts(QuiltRT);

	// Lenticular shader rendering
	bool bRenderLenticularShader = RenderingSettings.bRenderLenticularShader;
	ENQUEUE_RENDER_COMMAND(RenderLenticularShaderCommand)(
		[&, RenderContext, bRenderLenticularShader](FRHICommandListImmediate& RHICmdList)
	{
		SCOPE_CYCLE_COUNTER(STAT_RenderLenticularShader_RenderThread);
		if (bRenderLenticularShader)
		{
			HoloPlay::RenderLenticularShader_RenderThread(RHICmdList, RenderContext);
		}
	});

	// Draw Debug Parameters
	DrawDebugParameters(InCanvas);
}

bool FHoloPlayViewportClient::InputKey(FViewport * InViewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	GHoloPlayRuntime->OnHoloPlayInputKeyDelegate().Broadcast(InViewport, ControllerId, Key, EventType, AmountDepressed, bGamepad);

	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	// Process special input first
	if (Key == EKeys::Escape && EventType == EInputEvent::IE_Pressed)
	{
		IHoloPlayRuntime::Get().StopPlayer();
	}

	if (HoloPlaySettings->HoloPlayLenticularScreenshotSettings.InputKey == Key && EventType == EInputEvent::IE_Pressed)
	{
		PreparePlayLenticularScreenshot(HoloPlaySettings->HoloPlayLenticularScreenshotSettings.FileName, false, true);
	}

	if (HoloPlaySettings->HoloPlayScreenshotQuiltSettings.InputKey == Key && EventType == EInputEvent::IE_Pressed)
	{
		PreparePlayScreenshotQuilt(HoloPlaySettings->HoloPlayScreenshotQuiltSettings.FileName, true);
	}

	if (HoloPlaySettings->HoloPlayScreenshot2DSettings.InputKey == Key && EventType == EInputEvent::IE_Pressed)
	{
		PreparePlayScreenshot2D(HoloPlaySettings->HoloPlayScreenshot2DSettings.FileName, true);
	}

	if (HoloPlaySettings->HoloPlayDebugMenuSettings.InputKey == Key && EventType == EInputEvent::IE_Pressed)
	{
		UHoloPlaySettings* HoloPlaySettingsMutable = GetMutableDefault<UHoloPlaySettings>();
		HoloPlaySettingsMutable->HoloPlayDebugMenuSettings.ToggleDebugMode();
	}

	if (IgnoreInput())
	{
		return false;
	}

	bool bResult = false;

	// Make sure we are playing in separate window
	if (IHoloPlayRuntime::Get().GetCurrentHoloPlayModeType() == EHoloPlayModeType::PlayMode_InSeparateWindow)
	{
		// Make sure we are in game play mode
		if (GEngine->GameViewport != nullptr)
		{
			ULocalPlayer* FirstLocalPlayerFromController = GEngine->GameViewport->GetWorld()->GetFirstLocalPlayerFromController();

			UE_LOG(HoloPlayLogInput, Verbose, TEXT(">> InputKey %s, FirstLocalPlayerFromController %p, ControllerId %d"), *Key.ToString(), FirstLocalPlayerFromController, ControllerId);

			if (FirstLocalPlayerFromController && FirstLocalPlayerFromController->PlayerController)
			{
				bResult = FirstLocalPlayerFromController->PlayerController->InputKey(Key, EventType, AmountDepressed, bGamepad);
			}

			// A gameviewport is always considered to have responded to a mouse buttons to avoid throttling
			if (!bResult && Key.IsMouseButton())
			{
				bResult = true;
			}
		}
	}


	return bResult;
}

bool FHoloPlayViewportClient::InputAxis(FViewport * InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (IgnoreInput())
	{
		return false;
	}

	if (GWorld == nullptr || GEngine == nullptr || GEngine->GameViewport == nullptr || &GEngine->GameViewport->Viewport == nullptr)
	{
		return false;
	}

	bool bResult = false;

	// Don't allow mouse/joystick input axes while in PIE and the console has forced the cursor to be visible.  It's
	// just distracting when moving the mouse causes mouse look while you are trying to move the cursor over a button
	// in the editor!
	if (!(GEngine->GameViewport->Viewport->IsSlateViewport() && GEngine->GameViewport->Viewport->IsPlayInEditorViewport()) || GEngine->GameViewport->ViewportConsole == NULL || !GEngine->GameViewport->ViewportConsole->ConsoleActive())
	{
		// route to subsystems that care
		if (GEngine->GameViewport->ViewportConsole != NULL)
		{
			bResult = GEngine->GameViewport->ViewportConsole->InputAxis(ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
		}
		if (!bResult)
		{
			ULocalPlayer* const TargetPlayer = GEngine->GetLocalPlayerFromControllerId(GEngine->GameViewport, ControllerId);
			if (TargetPlayer && TargetPlayer->PlayerController)
			{
				UE_LOG(HoloPlayLogInput, Verbose, TEXT(">> FHoloPlayViewportClient::InputAxis"));
				bResult = TargetPlayer->PlayerController->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
			}
		}

		// For PIE, let the next PIE window handle the input if none of our players did
		// (this allows people to use multiple controllers to control each window)
		if (InViewport->IsPlayInEditorViewport())
		{
			UGameViewportClient *NextViewport = GEngine->GetNextPIEViewport(GEngine->GameViewport);
			if (NextViewport)
			{
				bResult = NextViewport->InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
			}
		}

		if (InViewport->IsSlateViewport() && InViewport->IsPlayInEditorViewport())
		{
			// Absorb all keys so game input events are not routed to the Slate editor frame
			bResult = true;
		}
	}

	return bResult;
}

bool FHoloPlayViewportClient::InputChar(FViewport * InViewport, int32 ControllerId, TCHAR Character)
{
	return false;
}

bool FHoloPlayViewportClient::InputTouch(FViewport * InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D & TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex)
{
	if (IgnoreInput())
	{
		return false;
	}

	if (GWorld == nullptr || GEngine == nullptr || GEngine->GameViewport == nullptr || &GEngine->GameViewport->Viewport == nullptr)
	{
		return false;
	}

	// route to subsystems that care
	bool bResult = (GEngine->GameViewport->ViewportConsole ? GEngine->GameViewport->ViewportConsole->InputTouch(ControllerId, Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchpadIndex) : false);
	if (!bResult)
	{
		ULocalPlayer* const TargetPlayer = GEngine->GetLocalPlayerFromControllerId(GEngine->GameViewport, ControllerId);
		if (TargetPlayer && TargetPlayer->PlayerController)
		{
			UE_LOG(HoloPlayLogInput, Verbose, TEXT(">> FHoloPlayViewportClient::InputTouch TouchLocation %s"), *TouchLocation.ToString());
			bResult = TargetPlayer->PlayerController->InputTouch(Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchpadIndex);
		}
	}

	return bResult;
}

bool FHoloPlayViewportClient::InputMotion(FViewport * InViewport, int32 ControllerId, const FVector & Tilt, const FVector & RotationRate, const FVector & Gravity, const FVector & Acceleration)
{
	return false;
}

void FHoloPlayViewportClient::RedrawRequested(FViewport * InViewport)
{
	Viewport->Draw();
}

bool FHoloPlayViewportClient::GetRenderTargetScreenShot(TWeakObjectPtr<UTextureRenderTarget2D> TextureRenderTarget2D, TArray<FColor>& Bitmap, const FIntRect& ViewRect)
{
	// Read the contents of the viewport into an array.
	auto ReadSurfaceDataFlags = FReadSurfaceDataFlags();
	ReadSurfaceDataFlags.SetLinearToGamma(false); // This is super important to disable this!

	bool bIsSuccess = false;
	FTextureRenderTargetResource* RenderTarget = TextureRenderTarget2D->GameThread_GetRenderTargetResource();
	if (RenderTarget->ReadPixels(Bitmap, ReadSurfaceDataFlags, ViewRect))
	{
		check(Bitmap.Num() == ViewRect.Area() || (Bitmap.Num() == TextureRenderTarget2D->SizeX * TextureRenderTarget2D->SizeY));
		for (FColor& color : Bitmap)
		{
			color.A = 255;
		}

		bIsSuccess = true;
	}

	return bIsSuccess;
}

static void ClipScreenshot(FIntVector& Size, FIntRect& SourceRect, TArray<FColor>& Bitmap)
{
	// Clip the bitmap to just the capture region if valid
	if (!SourceRect.IsEmpty())
	{
		FColor* const Data = Bitmap.GetData();
		const int32 OldWidth = Size.X;
		const int32 OldHeight = Size.Y;
		const int32 NewWidth = SourceRect.Width();
		const int32 NewHeight = SourceRect.Height();
		const int32 CaptureTopRow = SourceRect.Min.Y;
		const int32 CaptureLeftColumn = SourceRect.Min.X;

		for (int32 Row = 0; Row < NewHeight; Row++)
		{
			FMemory::Memmove(Data + Row * NewWidth, Data + (Row + CaptureTopRow) * OldWidth + CaptureLeftColumn, NewWidth * sizeof(*Data));
		}

		Bitmap.RemoveAt(NewWidth * NewHeight, OldWidth * OldHeight - NewWidth * NewHeight, false);
		Size = FIntVector(NewWidth, NewHeight, 0);
	}
}

void FHoloPlayViewportClient::ProcessScreenshotQuilts(UTextureRenderTarget2D* InQuiltRT)
{
	if (HoloPlayQuilScreenshotRequest.IsValid())
	{
		if (HoloPlayQuilScreenshotRequest->GetFilename().IsEmpty())
		{
			HoloPlayQuilScreenshotRequest.Reset();
			return;
		}

		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = GetRenderTargetScreenShot(InQuiltRT, Bitmap);
		FIntVector Size(InQuiltRT->SizeX, InQuiltRT->SizeY, 0);

		if (bScreenshotSuccessful)
		{
			FString ScreenShotName = HoloPlayQuilScreenshotRequest->GetFilename();

			int ScreenshotResolutionX = GetDefault<UHoloPlaySettings>()->HoloPlayScreenshotQuiltSettings.ScreenshotResolutionX;
			int ScreenshotResolutionY = GetDefault<UHoloPlaySettings>()->HoloPlayScreenshotQuiltSettings.ScreenshotResolutionY;

			if (ScreenshotResolutionX < 0) ScreenshotResolutionX = 0;
			if (ScreenshotResolutionY < 0) ScreenshotResolutionY = 0;

			FIntRect SourceRect(0, 0, ScreenshotResolutionX, ScreenshotResolutionY);
			ClipScreenshot(Size, SourceRect, Bitmap);

			if (!FPaths::GetExtension(ScreenShotName).IsEmpty())
			{
				ScreenShotName = FPaths::GetBaseFilename(ScreenShotName, false);
				ScreenShotName += TEXT(".png");
			}

			// Save the contents of the array to a png file.
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenShotName);
		}

		HoloPlayQuilScreenshotRequest.Reset();
		OnScreenshotQuiltRequestProcessed().Broadcast();
	}
}

void FHoloPlayViewportClient::ProcessScreenShots(FViewport * InViewport)
{
	if (HoloPlayLenticularScreenshotRequest.IsValid())
	{
		if (HoloPlayLenticularScreenshotRequest->GetFilename().IsEmpty())
		{
			HoloPlayLenticularScreenshotRequest.Reset();
			return;
		}

		TArray<FColor> Bitmap;
		bool bShowUI = false;
		TSharedPtr<SWindow> WindowPtr = GetWindow();
		if (HoloPlayLenticularScreenshotRequest->ShouldShowUI() && WindowPtr.IsValid())
		{
			bShowUI = true;
		}

		bool bScreenshotSuccessful = false;
		FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);
		if (bShowUI && FSlateApplication::IsInitialized())
		{
			TSharedRef<SWidget> WindowRef = WindowPtr.ToSharedRef();
			bScreenshotSuccessful = FSlateApplication::Get().TakeScreenshot(WindowRef, Bitmap, Size);
			GScreenshotResolutionX = Size.X;
			GScreenshotResolutionY = Size.Y;
		}
		else
		{
			bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);
		}

		if (bScreenshotSuccessful)
		{
			for (FColor& color : Bitmap)
			{
				color.A = 255;
			}

			FString ScreenShotName = HoloPlayLenticularScreenshotRequest->GetFilename();

			int ScreenshotResolutionX = GetDefault<UHoloPlaySettings>()->HoloPlayLenticularScreenshotSettings.ScreenshotResolutionX;
			int ScreenshotResolutionY = GetDefault<UHoloPlaySettings>()->HoloPlayLenticularScreenshotSettings.ScreenshotResolutionY;

			if (ScreenshotResolutionX < 0) ScreenshotResolutionX = 0;
			if (ScreenshotResolutionY < 0) ScreenshotResolutionY = 0;

			FIntRect SourceRect(0, 0, ScreenshotResolutionX, ScreenshotResolutionY);
			ClipScreenshot(Size, SourceRect, Bitmap);

			if (!FPaths::GetExtension(ScreenShotName).IsEmpty())
			{
				ScreenShotName = FPaths::GetBaseFilename(ScreenShotName, false);
				ScreenShotName += TEXT(".png");
			}

			// Save the contents of the array to a png file.
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenShotName);
		}

		HoloPlayLenticularScreenshotRequest.Reset();
		OnScreenshot3DRequestProcessed().Broadcast();
	}
}

EMouseCursor::Type FHoloPlayViewportClient::GetCursor(FViewport * InViewport, int32 X, int32 Y)
{
	return CurrentMouseCursor;
}

bool FHoloPlayViewportClient::IsFocused(FViewport* InViewport)
{
	return InViewport->HasFocus() || InViewport->HasMouseCapture();
}

void FHoloPlayViewportClient::LostFocus(FViewport * InViewport)
{
	CurrentMouseCursor = EMouseCursor::Default;
}

void FHoloPlayViewportClient::ReceivedFocus(FViewport * InViewport)
{
	CurrentMouseCursor = EMouseCursor::None;
}

void FHoloPlayViewportClient::ProcessScreenshot2D(TWeakObjectPtr<AHoloPlayCapture> HoloPlayCapture)
{
	if (HoloPlayScreenshot2DRequest.IsValid())
	{
		FString ScreenShotName = HoloPlayScreenshot2DRequest->GetFilename();

		if (ScreenShotName.IsEmpty())
		{
			HoloPlayScreenshot2DRequest.Reset();
			return;
		}

		auto TilingTextureRenderTarget2D = HoloPlayCapture->GetRenderCamera()->TextureTarget;
		int TextureOriginalX = TilingTextureRenderTarget2D->SizeX;
		int TextureOriginalY = TilingTextureRenderTarget2D->SizeY;
		bool bResChanged = false;


		int ScreenshotResolutionX = GetDefault<UHoloPlaySettings>()->HoloPlayScreenshot2DSettings.ScreenshotResolutionX;
		int ScreenshotResolutionY = GetDefault<UHoloPlaySettings>()->HoloPlayScreenshot2DSettings.ScreenshotResolutionY;
		FIntRect SourceRect(0, 0, ScreenshotResolutionX, ScreenshotResolutionY);

		if (!SourceRect.IsEmpty())
		{
			bResChanged = true;

			TilingTextureRenderTarget2D->ResizeTarget(ScreenshotResolutionX, ScreenshotResolutionY);
			TilingTextureRenderTarget2D->UpdateResourceImmediate();

			// Sync with rendering thread, in order to reset texure resolution
			FlushRenderingCommands();

			HoloPlayCapture->GetRenderCamera()->CaptureScene();
		}

		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = GetRenderTargetScreenShot(TilingTextureRenderTarget2D, Bitmap);
		FIntVector Size(TilingTextureRenderTarget2D->SizeX, TilingTextureRenderTarget2D->SizeY, 0);

		if (bScreenshotSuccessful)
		{
			if (!FPaths::GetExtension(ScreenShotName).IsEmpty())
			{
				ScreenShotName = FPaths::GetBaseFilename(ScreenShotName, false);
				ScreenShotName += TEXT(".png");
			}

			// Save the contents of the array to a png file.
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenShotName);
		}

		if (bResChanged)
		{
			TilingTextureRenderTarget2D->ResizeTarget(TextureOriginalX, TextureOriginalY);
			TilingTextureRenderTarget2D->UpdateResourceImmediate();

			// Sync with rendering thread, in order to reset texure resolution
			FlushRenderingCommands();
		}


		HoloPlayScreenshot2DRequest.Reset();
		OnScreenshot2DRequestProcessed().Broadcast();
	}
}


bool FHoloPlayViewportClient::Exec(UWorld * InWorld, const TCHAR * Cmd, FOutputDevice & Ar)
{
	if (FParse::Command(&Cmd, TEXT("HoloPlay.LenticularScreenshot")))
	{
		return HandleLenticularScreenshotCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.ScreenshotQuilt")))
	{
		return HandleScreenshotQuiltCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Screenshot2D")))
	{
		return HandleScreenshot2DCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Window")))
	{
		return HandleWindowCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Shader")))
	{
		return HandleShaderCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Scene")))
	{
		return HandleSceneCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Tilling")))
	{
		return HandleTillingCommand(Cmd, Ar);
	}
	else if (FParse::Command(&Cmd, TEXT("HoloPlay.Scalability")))
	{
		return HandleScalabilityCommand(Cmd, Ar);
	}
	else
	{
		return false;
	}
}

bool FHoloPlayViewportClient::HandleLenticularScreenshotCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	if (Viewport)
	{
		FString FileName;
		bool bAddFilenameSuffix = true;
		ParseScreenshotCommand(Cmd, FileName, bAddFilenameSuffix);

		return PreparePlayLenticularScreenshot(FileName, false, bAddFilenameSuffix);
	}
	return true;
}

bool FHoloPlayViewportClient::HandleScreenshotQuiltCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	if (Viewport)
	{
		FString FileName;
		bool bAddFilenameSuffix = true;
		ParseScreenshotCommand(Cmd, FileName, bAddFilenameSuffix);

		return PreparePlayScreenshotQuilt(FileName, bAddFilenameSuffix);
	}
	return true;
}

bool FHoloPlayViewportClient::HandleScreenshot2DCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	if (Viewport)
	{
		FString FileName;
		bool bAddFilenameSuffix = true;
		ParseScreenshotCommand(Cmd, FileName, bAddFilenameSuffix);

		return PreparePlayScreenshot2D(FileName, bAddFilenameSuffix);
	}
	return true;
}

bool FHoloPlayViewportClient::HandleWindowCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	bool bWasHandled = true;
	if (FParse::Command(&Cmd, TEXT("ClientSize")))
	{
		if (HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType == EHoloPlayModeType::PlayMode_InSeparateWindow)
		{
			uint32 X = 0; uint32 Y = 0; 
			auto WindowMode = HoloPlaySettings->HoloPlayWindowSettings.WindowType;
			ParseResolution(Cmd, X, Y, WindowMode);

			if (X > 0 && Y > 0)
			{
				HoloPlaySettings->HoloPlayWindowSettings.ClientSize = FVector2D(X, Y);
			}
			HoloPlaySettings->HoloPlayWindowSettings.WindowType = WindowMode;

			// Restart player
			IHoloPlayRuntime::Get().RestartPlayer(HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType);
		}
		else
		{
			bWasHandled = false;
		}
	}
	if (FParse::Command(&Cmd, TEXT("AutoPlacement")))
	{
		if (FString(Cmd).IsNumeric())
		{
			int32 NewVal = FCString::Atoi(*FString(Cmd));
			HoloPlaySettings->HoloPlayWindowSettings.bAutoPlacementInHoloPlay = !!NewVal;
		}
		else bWasHandled = false;
	}
	if (FParse::Command(&Cmd, TEXT("WindowMode")))
	{
		if (FString(Cmd).IsNumeric())
		{
			int32 NewVal = FCString::Atoi(*FString(Cmd));
			
			EWindowMode::Type WinMode = static_cast<EWindowMode::Type>(NewVal);
			EHoloPlayWindowType HoloPlayWinMode = static_cast<EHoloPlayWindowType>(NewVal);

			if (WinMode != GetWindow()->GetWindowMode())
			{
				GetWindow()->SetWindowMode(WinMode);
				HoloPlaySettings->HoloPlayWindowSettings.WindowType = HoloPlayWinMode;
			}
			else bWasHandled = false;
		}
		else bWasHandled = false;
	}
	else
	{
		bWasHandled = false;
	}

	if (bWasHandled)
	{
		HoloPlaySettings->HoloPlaySave();
	}

	return bWasHandled;

	return true;
}

bool FHoloPlayViewportClient::HandleShaderCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	bool bWasHandled = true;
	if (FParse::Command(&Cmd, TEXT("QuiltMode")))
	{
		int32 NewVal = FCString::Atoi(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.QuiltMode = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("Pitch")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.Pitch.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("Center")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.center.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("ViewCone")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.viewCone.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("ViewInversion")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.invView.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("VertAngle")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.verticalAngle.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("DPI")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.DPI.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("FlipImageX")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.flipImageX.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("FlipImageY")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.flipImageY.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("FlipSubp")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.flipSubp.DefaultValue = NewVal;
	}
	else if (FParse::Command(&Cmd, TEXT("DefaultAspect")))
	{
		float NewVal = FCString::Atof(*FString(Cmd));
		HoloPlaySettings->HoloPlayShaderSettings.DefaultAspect.DefaultValue = NewVal;
	}
	else
	{
		bWasHandled = false;
	}

	if (bWasHandled)
	{
		HoloPlaySettings->HoloPlaySave();
	}

	return bWasHandled;
}

bool FHoloPlayViewportClient::HandleSceneCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	return true;
}

bool FHoloPlayViewportClient::HandleTillingCommand(const TCHAR* Cmd, FOutputDevice& Ar)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	auto GameHoloPlayCapture = GetGameHoloPlayCapture();

	bool bWasHandled = true;
	if (FParse::Command(&Cmd, TEXT("Standard")))
	{
		GameHoloPlayCapture->UpdateTillingProperties(EHoloPlayQualitySettings::Q_Standard);
	}
	else if (FParse::Command(&Cmd, TEXT("Default")))
	{
		GameHoloPlayCapture->UpdateTillingProperties(EHoloPlayQualitySettings::Q_Default);
	}
	else if (FParse::Command(&Cmd, TEXT("HighRes")))
	{
		GameHoloPlayCapture->UpdateTillingProperties(EHoloPlayQualitySettings::Q_High_Res);
	}
	else if (FParse::Command(&Cmd, TEXT("HighView")))
	{
		GameHoloPlayCapture->UpdateTillingProperties(EHoloPlayQualitySettings::Q_High_View);
	}
	else if (FParse::Command(&Cmd, TEXT("ExtraLow")))
	{
		GameHoloPlayCapture->UpdateTillingProperties(EHoloPlayQualitySettings::Q_Extra_Low);
	}
	else
	{
		bWasHandled = false;
	}

	if (bWasHandled)
	{
		HoloPlaySettings->HoloPlaySave();
	}

	return bWasHandled;
}

bool FHoloPlayViewportClient::HandleScalabilityCommand(const TCHAR * Cmd, FOutputDevice & Ar)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	int32 NewVal = FCString::Atoi(*FString(Cmd));
	HoloPlaySettings->Scalability = NewVal;

	auto HoloPlayScalabilityManager = GHoloPlayRuntime->GetHoloPlayScalabilityManager();
	if (HoloPlayScalabilityManager.IsValid())
	{
		HoloPlayScalabilityManager->SetScalability(NewVal);
	}

	HoloPlaySettings->HoloPlaySave();

	return true;
}

void FHoloPlayViewportClient::DrawDebugText(FCanvas * InCanvas, FVector Location, FHoloPlayConfigValues Value, FColor Color, float Scale)
{
	FString DebugString = FString::Printf(TEXT("%s: %f"), *Value.Name, Value.DefaultValue);
	UFont * DebugFont = (GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugFont) ? GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugFont : GEngine->GetLargeFont();

	if (InCanvas && IHoloPlayRuntime::Get().GetCurrentHoloPlayModeType() == EHoloPlayModeType::PlayMode_InSeparateWindow)
	{
		auto HoloPlayDebugMenuSettings = GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings;

		// Make Text Item for FCanvas
		FCanvasTextItem TextItem(FVector2D(Location.X, Location.Y), FText::FromString(DebugString), DebugFont, Color);
		FCanvasTileItem TileItem(
			FVector2D(Location.X - 10.0f, Location.Y + (12.0f * Scale)),
			FVector2D(
				HoloPlayDebugMenuSettings.DebugTileWidth * Scale,
				HoloPlayDebugMenuSettings.DebugTileHeight * Scale),
			    HoloPlayDebugMenuSettings.DebugBackgroundColor
		);
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.BlendMode = SE_BLEND_Translucent;

		// Define Rendering Info
		FFontRenderInfo FRI = FFontRenderInfo();
		FRI.bClipText = true;
		TextItem.FontRenderInfo = FRI;

		// Draw Text To Screen
		InCanvas->DrawItem(TileItem);
		InCanvas->DrawItem(TextItem);
	}
}

void FHoloPlayViewportClient::DrawDebugParameters(FCanvas * InCanvas)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawDebugParameters_GameThread);

	if (GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.debugMode && InCanvas != nullptr)
	{
		UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
		FVector DebugLocation = GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugFontLocation;
		float DebugScale = GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugFontScaleLG;
		FColor FontColor = GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugFontColor;
		FVector DebugScaleVector = FVector(0.0f, DebugScale, 0.0f);

		//Add padding for background tile
		FCanvasTileItem TileItem1(FVector2D(DebugLocation.X - 10.0f, DebugLocation.Y + (2.0f*DebugScale)), FVector2D(GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugTileWidth * DebugScale, GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugTileHeight * DebugScale), GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugBackgroundColor);
		FCanvasTileItem TileItem2(FVector2D(DebugLocation.X - 10.0f, DebugLocation.Y - (8.0f*DebugScale)), FVector2D(GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugTileWidth * DebugScale, GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugTileHeight * DebugScale), GetDefault<UHoloPlaySettings>()->HoloPlayDebugMenuSettings.DebugBackgroundColor);
		InCanvas->DrawItem(TileItem1);
		InCanvas->DrawItem(TileItem2);

		// Draw Debug to HoloPlay Viewport
		DrawDebugText(InCanvas, DebugLocation, HoloPlaySettings->HoloPlayShaderSettings.Pitch, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 10.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.slope, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 20.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.center, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 30.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.viewCone, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 40.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.invView, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 50.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.verticalAngle, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 60.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.DPI, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 70.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.flipImageX, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 80.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.flipImageY, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 90.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.flipSubp, FontColor, DebugScale);
		DrawDebugText(InCanvas, DebugLocation + DebugScale * FVector(0.0f, 100.0f, 0.0f), HoloPlaySettings->HoloPlayShaderSettings.DefaultAspect, FontColor, DebugScale);
	}
}

void FHoloPlayViewportClient::ParseScreenshotCommand(const TCHAR * Cmd, FString& InName, bool& InSuffix)
{
	FString CmdString(Cmd);
	TArray<FString> Args;
	CmdString.ParseIntoArray(Args, TEXT(" "));
	if (Args.Num() > 1)
	{
		InName = Args[0];
	}
	else
	{
		InName = CmdString;
	}

	if (FParse::Param(Cmd, TEXT("nosuffix")))
	{
		InSuffix = false;
	}
}

bool FHoloPlayViewportClient::ParseResolution(const TCHAR * InResolution, uint32 & OutX, uint32 & OutY, EHoloPlayWindowType & OutWindowMode)
{
	if (*InResolution)
	{
		FString CmdString(InResolution);
		CmdString = CmdString.TrimStartAndEnd().ToLower();

		//Retrieve the X dimensional value
		const uint32 X = FMath::Max(FCString::Atof(*CmdString), 0.0f);

		// Determine whether the user has entered a resolution and extract the Y dimension.
		FString YString;

		// Find separator between values (Example of expected format: 1280x768)
		const TCHAR* YValue = NULL;
		if (FCString::Strchr(*CmdString, 'x'))
		{
			YValue = const_cast<TCHAR*> (FCString::Strchr(*CmdString, 'x') + 1);
			YString = YValue;
			// Remove any whitespace from the end of the string
			YString = YString.TrimStartAndEnd();
		}

		// If the Y dimensional value exists then setup to use the specified resolution.
		uint32 Y = 0;
		if (YValue && YString.Len() > 0)
		{
			// See if there is a fullscreen flag on the end
			FString FullScreenChar = YString.Mid(YString.Len() - 1);
			FString WindowFullScreenChars = YString.Mid(YString.Len() - 2);
			EHoloPlayWindowType WindowMode = OutWindowMode;
			if (!FullScreenChar.IsNumeric())
			{
				int StringTripLen = 0;

				if (WindowFullScreenChars == TEXT("wf"))
				{
					WindowMode = EHoloPlayWindowType::WindowedFullscreen;
					StringTripLen = 2;
				}
				else if (FullScreenChar == TEXT("f"))
				{
					WindowMode = EHoloPlayWindowType::Fullscreen;
					StringTripLen = 1;
				}
				else if (FullScreenChar == TEXT("w"))
				{
					WindowMode = EHoloPlayWindowType::Windowed;
					StringTripLen = 1;
				}

				YString = YString.Left(YString.Len() - StringTripLen).TrimStartAndEnd();
			}

			if (YString.IsNumeric())
			{
				Y = FMath::Max(FCString::Atof(YValue), 0.0f);
				OutX = X;
				OutY = Y;
				OutWindowMode = WindowMode;
				return true;
			}
		}
	}
	return false;
}

bool FHoloPlayViewportClient::PreparePlayLenticularScreenshot(const FString& FileName, bool bInShowUI, bool bAddFilenameSuffix)
{
	if (!HoloPlayLenticularScreenshotRequest.IsValid())
	{
		HoloPlayLenticularScreenshotRequest = MakeShareable(new FHoloPlayLenticularScreenshotRequest());
		HoloPlayLenticularScreenshotRequest->RequestScreenshot(FileName, bInShowUI, bAddFilenameSuffix);

		return true;
	}

	return false;
}

bool FHoloPlayViewportClient::PreparePlayScreenshotQuilt(const FString& FileName, bool bAddFilenameSuffix)
{
	if (!HoloPlayQuilScreenshotRequest.IsValid())
	{
		HoloPlayQuilScreenshotRequest = MakeShareable(new FHoloPlayScreenshotRequest());
		HoloPlayQuilScreenshotRequest->RequestScreenshot(FileName, bAddFilenameSuffix);

		return true;
	}

	return false;
}

bool FHoloPlayViewportClient::PreparePlayScreenshot2D(const FString& FileName, bool bAddFilenameSuffix)
{
	if (!HoloPlayScreenshot2DRequest.IsValid())
	{
		HoloPlayScreenshot2DRequest = MakeShareable(new FHoloPlayScreenshotRequest());
		HoloPlayScreenshot2DRequest->RequestScreenshot(FileName, bAddFilenameSuffix);

		return true;
	}

	return false;
}

TWeakObjectPtr<AHoloPlayCapture> FHoloPlayViewportClient::GetGameHoloPlayCapture()
{
#if WITH_EDITOR
	return IHoloPlayRuntime::Get().GameHoloPlayCapture.IsValid() ? IHoloPlayRuntime::Get().GameHoloPlayCapture : IHoloPlayRuntime::Get().EditorHoloPlayCapture;
#else
	return IHoloPlayRuntime::Get().GameHoloPlayCapture;
#endif // WITH_EDITOR
}

UTextureRenderTarget2D* FHoloPlayViewportClient::GetQuiltRT(TWeakObjectPtr<AHoloPlayCapture> HoloPlayCapture)
{
	if (QuiltRT == nullptr)
	{
		QuiltRT = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), UTextureRenderTarget2D::StaticClass());
		QuiltRT->InitCustomFormat(HoloPlayCapture->GetTilingValues().QuiltW, HoloPlayCapture->GetTilingValues().QuiltH, PF_A16B16G16R16, false);
		QuiltRT->ClearColor = FLinearColor::Red;
		QuiltRT->UpdateResourceImmediate();
		QuiltRT->AddToRoot();
	}

	// Resize Quilt texture
	if (HoloPlayCapture->GetTilingValues().QuiltW != QuiltRT->SizeX ||
		HoloPlayCapture->GetTilingValues().QuiltH != QuiltRT->SizeY)
	{
		QuiltRT->ResizeTarget(HoloPlayCapture->GetTilingValues().QuiltW, HoloPlayCapture->GetTilingValues().QuiltH);
		QuiltRT->UpdateResourceImmediate();
	}


	return QuiltRT;
}

