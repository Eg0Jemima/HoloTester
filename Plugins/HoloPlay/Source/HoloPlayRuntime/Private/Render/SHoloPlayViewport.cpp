#include "Render/SHoloPlayViewport.h"

#include "Render/HoloPlayViewportClient.h"


#include "Misc/HoloPlayLog.h"

#include "IHoloPlayRuntime.h"

#include "SceneViewport.h"
#include "SViewport.h"
#include "SToolTip.h"
#include "Engine.h"

void SHoloPlayViewport::Construct(const FArguments& InArgs)
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	// If add to main viewport should be false
	RenderDirectlyToWindow = InArgs._RenderDirectlyToWindow;

	// Create Viewport Widget
	Viewport = SNew(SViewport)
		.IsEnabled(true)
		.ShowEffectWhenDisabled(false)
		.RenderDirectlyToWindow(RenderDirectlyToWindow) // Make same like in Unity, it will be dark scene but same look 
		.EnableGammaCorrection(false) // FIX dark color, should be false. Whether or not to enable gamma correction.Doesn't apply when rendering directly to a backbuffer.
		.ReverseGammaCorrection(false)
		.EnableBlending(false)
		.EnableStereoRendering(false)
		.PreMultipliedAlpha(true)
		.IgnoreTextureAlpha(true);

	// Create Viewport Client
	HoloPlayViewportClient = MakeShareable(new FHoloPlayViewportClient());

	// Create Scene Viewport
	SceneViewport = MakeShareable(new FSceneViewport(HoloPlayViewportClient.Get(), Viewport));

	// Set Viewport 
	HoloPlayViewportClient->Viewport = SceneViewport.Get();
	HoloPlayViewportClient->HoloPlaySceneViewport = SceneViewport.Get();

	// Assign SceneViewport to Viewport widget. Needed for rendering
	Viewport->SetViewportInterface(SceneViewport.ToSharedRef());

	// Assign Viewport widget for our custom PlayScene Viewport
	this->ChildSlot
		[
			Viewport.ToSharedRef()
		];
}

void SHoloPlayViewport::Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Call FViewport each engine tick
	SceneViewport->Draw();
}
