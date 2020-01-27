#include "Slate/HoloPlayProSceneViewport.h"

#include "Framework/Application/SlateApplication.h"

FHoloPlayProSceneViewport::FHoloPlayProSceneViewport(FViewportClient * InViewportClient, TSharedPtr<SViewport> InViewportWidget)
	: FSceneViewport(InViewportClient, InViewportWidget)
{
}

FHoloPlayProSceneViewport::~FHoloPlayProSceneViewport()
{
}
