#pragma once

#include "CoreMinimal.h"
#include "Widgets/SViewport.h"

class UHoloPlayProViewportClient;
class FHoloPlayProSceneViewport;

class SHoloPlayProViewport : public SViewport
{

	SLATE_BEGIN_ARGS(SHoloPlayProViewport)
	{
	}
	SLATE_END_ARGS()

	SHoloPlayProViewport();
	~SHoloPlayProViewport();

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
	UHoloPlayProViewportClient* GetHoloPlayProViewportClient() { return HoloPlayProViewportClient; }
	virtual void Release();

public:
	UHoloPlayProViewportClient* HoloPlayProViewportClient;
	TSharedPtr<FHoloPlayProSceneViewport> HoloPlayProSceneViewport;
};


