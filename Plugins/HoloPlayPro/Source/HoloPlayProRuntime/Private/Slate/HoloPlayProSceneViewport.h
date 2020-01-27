#pragma once

#include "CoreMinimal.h"
#include "Slate/SceneViewport.h"

class SViewport;
class FViewportClient;

class FHoloPlayProSceneViewport : public FSceneViewport
{
public:
	FHoloPlayProSceneViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget);
	virtual ~FHoloPlayProSceneViewport();
};