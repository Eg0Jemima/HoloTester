// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/HoloPlayCapture.h"
#include "Game/HoloPlayCameraComponent.h"

#include "Misc/HoloPlayLog.h"
#include "Misc/HoloPlayStats.h"

#include "IHoloPlayRuntime.h"

#include "GameFramework/SpringArmComponent.h"
#include "Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
AHoloPlayCapture::AHoloPlayCapture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Camera Boom for Camera Visible in Editor Setup
	PreviewCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	PreviewCameraSpringArm->SetupAttachment(RootComponent);
	PreviewCameraSpringArm->bDoCollisionTest = false;

	// Camera Visible in Editor Setup
	PreviewCamera = CreateDefaultSubobject<UHoloPlayCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(PreviewCameraSpringArm, USpringArmComponent::SocketName);

	// Render Camera Setup
	RenderCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("RenderCamera"));
	RenderCamera->SetupAttachment(PreviewCamera);

	// Create tiling texture
	const FName TargetName = MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), TEXT("SceneCaptureTextureTarget"));
	UTextureRenderTarget2D* TilingRenderTarget = NewObject<UTextureRenderTarget2D>(this, TargetName);
	TilingRenderTarget->InitCustomFormat(1280, 720, PF_A16B16G16R16, false); // Resolution will be changes later, dynamically
	TilingRenderTarget->ClearColor = FLinearColor::Red;
	TilingRenderTarget->UpdateResourceImmediate();

	RenderCamera->TextureTarget = TilingRenderTarget;
	RenderCamera->SetHiddenInGame(false);
	RenderCamera->bCaptureEveryFrame = false;
	RenderCamera->bUseCustomProjectionMatrix = true;
	RenderCamera->bAlwaysPersistRenderingState = true;
	RenderCamera->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// Init Config Values
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();
	TilingValues = HoloPlaySettings->DefaultSettings;
	CustomTilingValues = HoloPlaySettings->CustomSettings;
	VerticalAngle = HoloPlaySettings->HoloPlayShaderSettings.verticalAngle;
	ViewCone = HoloPlaySettings->HoloPlayShaderSettings.viewCone;
	Aspect = HoloPlaySettings->HoloPlayShaderSettings.GetAspect();
	ResetCam();
}

// Called when the game starts or when spawned
void AHoloPlayCapture::BeginPlay()
{
	Super::BeginPlay();

	ResetCam();

	ResizeTilingTexture(RenderCamera->TextureTarget);
	IHoloPlayRuntime::Get().GameHoloPlayCapture = this;
}

// Called when the game ends
void AHoloPlayCapture::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	IHoloPlayRuntime::Get().GameHoloPlayCapture = this;
}

void AHoloPlayCapture::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	SetRenderingFeatures();

	ResetCam();

	if (GIsEditor)
	{
		ResizeTilingTexture(RenderCamera->TextureTarget);
		IHoloPlayRuntime::Get().EditorHoloPlayCapture = this;
	}
}

void AHoloPlayCapture::SetRenderingFeatures()
{
	if (EnableAdvancedRenderingFeatures)
	{
		RenderCamera->ShowFlags.EnableAdvancedFeatures();
	}
	else
	{
		RenderCamera->ShowFlags.DisableAdvancedFeatures();
	}

	RenderCamera->ShowFlags.SetTemporalAA(false);
}

// Recalculates tiling values and sets new values used for frustum visualization
// Called on OnConstruction and whenever any values are modified.
void AHoloPlayCapture::ResetCam()
{
	float vertAngle = (bUseCustomVerticalAngle) ? CustomVerticalAngle: VerticalAngle.DefaultValue;
	TilingValues.Setup();

	ResetCam(vertAngle, FOV);
}

void AHoloPlayCapture::ResetCam(float InVerticalAngle, float fov, bool resetOffset)
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	float CamDistance = GetCamDistance();

	float NearClipMax = (NearClipFactor*Size > CamDistance) ? CamDistance / Size : NearClipFactor;
	NearClipFactor = (NearClipFactor < 0.0f) ? 0.0f : NearClipMax;
	FarClipFactor = (FarClipFactor < 0.0f) ? 0.0f : FarClipFactor;

	// Clipping Plane Distance
	NearClipPlane = CamDistance - NearClipFactor * Size;
	if (NearClipPlane <= NearClipMin)
	{
		NearClipPlane = NearClipMin;
	}
	FarClipPlane = CamDistance + FarClipFactor * Size;

	// Preview Camera Distance
	PreviewCameraSpringArm->TargetArmLength = CamDistance;

#if WITH_EDITOR
	// Preview Camera Setup - Pass data to camera component and draw frustum component
	PreviewCamera->NearClipPlane = NearClipPlane;
	PreviewCamera->FarClipPlane = FarClipPlane;
	PreviewCamera->HoloPlayPlane = CamDistance;
	PreviewCamera->AspectRatio = HoloPlaySettings->HoloPlayShaderSettings.GetAspect();
	PreviewCamera->FieldOfView = fov;
	PreviewCamera->VertAngle = InVerticalAngle;
	PreviewCamera->MidPlaneLineLength = MidPlaneLineLength;
	PreviewCamera->FrustumLineThickness = FrustumLineThickness;
	PreviewCamera->MidLineThickness = MidLineThickness;
	PreviewCamera->ProjectionMode = ECameraProjectionMode::Perspective;

	PreviewCamera->OverrideFrustumColor(FrustumColor, MidPlaneFrustumColor);
	PreviewCamera->RefreshVisualRepresentation();
#endif
}

void AHoloPlayCapture::ResizeTilingTexture(UTextureRenderTarget2D * TextureRenderTarget2D)
{
	if (TextureRenderTarget2D != nullptr)
	{
		// Resize Tiling render target size
		if (TextureRenderTarget2D->SizeX != TilingValues.TileSizeX ||
			TextureRenderTarget2D->SizeY != TilingValues.TileSizeY)
		{
			TextureRenderTarget2D->ResizeTarget(TilingValues.TileSizeX, TilingValues.TileSizeY);
			TextureRenderTarget2D->UpdateResourceImmediate();
		}
	}
}

#if WITH_EDITOR
void AHoloPlayCapture::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

	if (PropertyName.ToString() == "TilingQuality")
	{
		UpdateTillingProperties(TilingQuality);
	}
	if (PropertyName.ToString() == "FrustumColor")
	{
		PreviewCamera->OverrideFrustumColor(FrustumColor, MidPlaneFrustumColor);
	}
	
	// @TODO. For some reason it works only in GameMode
	if (PropertyName.ToString() == "EnableAdvancedRenderingFeatures")
	{
		SetRenderingFeatures();
	}
	// Check if Game is running
	if (AHoloPlayCapture::GetWorld() && AHoloPlayCapture::GetWorld()->IsGameWorld())
	{
		ResetCam();
	}
	Super::PostEditChangeProperty(e);
}
#endif

void AHoloPlayCapture::UpdateTillingProperties(EHoloPlayQualitySettings TilingSettings)
{
	if (TilingValues.Text == "Custom")
	{
		CustomTilingValues = TilingValues;
	}
	switch (TilingSettings)
	{
		case EHoloPlayQualitySettings::Q_Standard:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->StandardSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_Default:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->DefaultSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_High_Res:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->HighResSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_High_View:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->HighViewSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_Extra_Low:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->ExtraLowSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_Ultra_Res:
		{
			TilingValues = GetDefault<UHoloPlaySettings>()->UltraResSettings;
			return;
		}
		case EHoloPlayQualitySettings::Q_Custom:
		{
			TilingValues = CustomTilingValues;
			return;
		}
	}
}

//Called by HoloPlayViewportClient used for capturing new snapshot of scene from SceneCapture (RenderCamera)
void AHoloPlayCapture::RenderView(int ViewToRender, int NumViewsToRender)
{
	if (bIsSimpleRender)
	{
		float CamDistance = GetCamDistance();
		float ViewConeSweep = CamDistance * FMath::Tan(FMath::DegreesToRadians(ViewCone.DefaultValue));

		float CurrentViewLerp = 0.f; // if numviews is 1, take center view
		if (NumViewsToRender > 1)
		{
			CurrentViewLerp = (float)ViewToRender / (NumViewsToRender - 1.f) - .5f;
		}

		float ViewOffsetX = CurrentViewLerp * ViewConeSweep;
		float ProjModifier = 1.0f / Size;
		float ProjOffsetX = ViewOffsetX * ProjModifier;

		UE_LOG(HoloPlayLogGame, Verbose, TEXT("ViewOffsetX: %f, ProjOffsetX: %f"), ViewOffsetX, ProjOffsetX);

		RenderCamera->SetRelativeLocation(FVector(0.0f, ViewOffsetX, 0.0f));
		RenderCamera->CustomProjectionMatrix = GenerateProjectionMatrix(FOV, NearClipPlane, FarClipPlane, ProjOffsetX, 0.f);

		SCOPE_CYCLE_COUNTER(STAT_CaptureScene_GameThread);
		RenderCamera->CaptureScene();
	}
	// @TODO. Rendering with AngleAtView will be removed.
	else
	{
		float vertAngle = (bUseCustomVerticalAngle) ? CustomVerticalAngle : VerticalAngle.DefaultValue;
		float viewCone = (bUseCustomViewCone) ? CustomViewCone : ViewCone.DefaultValue;
		HandleOffset(AngleAtView(ViewToRender, NumViewsToRender, viewCone), vertAngle);
	}
}

// Calculate view and projection offsets
void AHoloPlayCapture::HandleOffset(float InHorizontalAngle, float InVerticalAngle)
{
	float CamDistance = GetCamDistance();
	float OffsetX = CamDistance * FMath::Tan(FMath::DegreesToRadians(InHorizontalAngle));
	float OffsetY = CamDistance * FMath::Tan(FMath::DegreesToRadians(InVerticalAngle));

	// Same as Tan(Angle) / Tan(HalfFOV)
	float ProjOffsetX = OffsetX / Size;
	float ProjOffsetY = OffsetY / Size;

	UE_LOG(HoloPlayLogGame, Verbose, TEXT("InHorizontalAngle: %f"), InHorizontalAngle);
	UE_LOG(HoloPlayLogGame, Verbose, TEXT("InVerticalAngle: %f"), InVerticalAngle);
	UE_LOG(HoloPlayLogGame, Verbose, TEXT("OffsetX: %f"), OffsetX);
	UE_LOG(HoloPlayLogGame, Verbose, TEXT("OffsetY %f, Aspect %f, OffsetY / Aspect: %f"), OffsetY, Aspect, OffsetY / Aspect);
	UE_LOG(HoloPlayLogGame, Verbose, TEXT("ProjOffsetX: %f, ProjOffsetY %f"), ProjOffsetX, ProjOffsetY);

	RenderCamera->SetRelativeLocation(FVector(0.0f, OffsetX, OffsetY / Aspect));
	RenderCamera->CustomProjectionMatrix = GenerateProjectionMatrix(FOV, NearClipPlane, FarClipPlane, ProjOffsetX, ProjOffsetY);

	SCOPE_CYCLE_COUNTER(STAT_CaptureScene_GameThread);
	RenderCamera->CaptureScene();
}


//Calculate the proper horizontal angle to use for offset for particular view
float AHoloPlayCapture::AngleAtView(int View, int NumViews, float InViewCone)
{
	InViewCone = FMath::Abs(InViewCone);

	if (NumViews <= 1)
	{
		return 0;
	}
	//(float)ViewToRender / (NumViewsToRender - 1.f) - .5f;
	float AngleAtView = -InViewCone * 0.5f + (float)View / (NumViews - 1.0f) * InViewCone;
	UE_LOG(HoloPlayLogGame, Verbose, TEXT("AngleAtView: %f"), AngleAtView);
	return AngleAtView;
}


//Get distance between the render camera and the center of the capture area
float AHoloPlayCapture::GetCamDistance()
{
	return Size / FMath::Tan(FMath::DegreesToRadians(FOV * 0.5f));
}


// Calculate and return matrix used for camera projection matrices
FMatrix AHoloPlayCapture::GenerateProjectionMatrix(float INFOV, float Near, float Far, float OffsetX, float OffsetY)
{
	float HalfFOV = FMath::DegreesToRadians(INFOV) / 2.0f;
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	float Width = HoloPlaySettings->HoloPlayShaderSettings.ScreenWidth.DefaultValue;
	float Height = HoloPlaySettings->HoloPlayShaderSettings.ScreenHeight.DefaultValue;

	float MaxZ = 0.f;
	float MinZ = 0.f;
	if (bUseFarClipPlane)
	{
		MaxZ = ((Near == Far) ? 0.0f : Near / (Near - Far));
		MinZ = ((Near == Far) ? Near : -Far * Near / (Near - Far));
	}
	else
	{
		MinZ = Near;
	}

	FMatrix PerspectiveMatrix(
		FPlane(1.0f / FMath::Tan(HalfFOV),	0.0f,									0.0f,	0.0f),
		FPlane(0.0f,						Width / FMath::Tan(HalfFOV) / Height,	0.0f,	0.0f),
		FPlane(OffsetX,						OffsetY,								MaxZ,	1.0f),
		FPlane(0.0f,						0.0f,									MinZ,	0.0f)
	);

	UE_LOG(HoloPlayLogGame, Verbose, TEXT("PerspectiveMatrix: %s"), *PerspectiveMatrix.ToString());

	return PerspectiveMatrix;
}

// Reset view camera locations
void AHoloPlayCapture::ResetCameras()
{
	HandleOffset(ResetCamerasView, VerticalAngle.DefaultValue);
}
