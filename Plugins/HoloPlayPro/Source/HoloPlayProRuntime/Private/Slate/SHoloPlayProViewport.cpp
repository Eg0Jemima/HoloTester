
#include "Slate/SHoloPlayProViewport.h"
#include "Slate/HoloPlayProViewportClient.h"
#include "Slate/HoloPlayProSceneViewport.h"
#include "Slate/SGameLayerManager.h"

#include "Framework/Application/SlateApplication.h"
#include "Engine.h"

SHoloPlayProViewport::SHoloPlayProViewport()
	: HoloPlayProViewportClient(nullptr)
{

}

SHoloPlayProViewport::~SHoloPlayProViewport()
{

}

void SHoloPlayProViewport::Construct(const FArguments & InArgs)
{
	TSharedRef<SOverlay> ViewportOverlayWidgetRef = SNew(SOverlay);
	TSharedRef<SGameLayerManager> GameLayerManagerRef = SNew(SGameLayerManager)
		[
			ViewportOverlayWidgetRef
		];


	SViewport::FArguments ParentArgs;
	ParentArgs.IgnoreTextureAlpha(false);
	ParentArgs.EnableBlending(false);
	//ParentArgs.RenderDirectlyToWindow(true);
	SViewport::Construct(ParentArgs);
	this->ChildSlot
		[
			GameLayerManagerRef
		];

	UHoloPlayProViewportClient* ViewportClient = NULL;

	ViewportClient = NewObject<UHoloPlayProViewportClient>(GEngine, UHoloPlayProViewportClient::StaticClass());
	ViewportClient->Init();
	ViewportClient->SetViewportOverlayWidget(ViewportOverlayWidgetRef);
	ViewportClient->SetGameLayerManager(GameLayerManagerRef);

	HoloPlayProSceneViewport = MakeShareable(new FHoloPlayProSceneViewport(ViewportClient, SharedThis(this)));
	HoloPlayProViewportClient = ViewportClient;
	HoloPlayProViewportClient->AddToRoot();

	// The viewport widget needs an interface so it knows what should render
	SetViewportInterface(HoloPlayProSceneViewport.ToSharedRef());
}

void SHoloPlayProViewport::Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	HoloPlayProSceneViewport->Invalidate();

	HoloPlayProSceneViewport->Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

void SHoloPlayProViewport::Release()
{
	if (HoloPlayProViewportClient != nullptr)
	{
		HoloPlayProViewportClient->RemoveFromRoot();
		HoloPlayProViewportClient->ClearFlags(RF_Standalone | RF_Public);
		HoloPlayProViewportClient->MarkPendingKill();
		HoloPlayProViewportClient = nullptr;
	}

	HoloPlayProSceneViewport.Reset();

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

}
