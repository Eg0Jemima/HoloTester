#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "Engine/ScriptViewportClient.h"
#include "Widgets/SWidget.h"
#include "Widgets/SOverlay.h"

#include "HoloPlayProViewportClient.generated.h"

class IGameLayerManager;
class ULocalPlayer;

UCLASS(config = Engine)
class UHoloPlayProViewportClient : public UObject, public FCommonViewportClient
{
	GENERATED_UCLASS_BODY()
public:
	virtual ~UHoloPlayProViewportClient();

	virtual void Init();

	virtual void Tick(float DeltaTime);

	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual UWorld* GetWorld() const override;
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false) override;
	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;
	virtual bool InputTouch(FViewport* Viewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex) override;
	virtual bool InputGesture(FViewport* Viewport, EGestureEvent GestureType, const FVector2D& GestureDelta, bool bIsDirectionInvertedFromDevice) override;
	virtual bool InputMotion(FViewport* Viewport, int32 ControllerId, const FVector& Tilt, const FVector& RotationRate, const FVector& Gravity, const FVector& Acceleration) override;


	virtual bool AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder = 0);
	virtual void RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent);
	virtual void AddViewportWidgetForPlayer(ULocalPlayer* Player, TSharedRef<SWidget> ViewportContent, const int32 ZOrder);
	virtual void RemoveViewportWidgetForPlayer(ULocalPlayer* Player, TSharedRef<SWidget> ViewportContent);
	void RemoveAllViewportWidgets();

	/** Assigns the viewport overlay widget to use for this viewport client.  Should only be called when first created */
	void SetViewportOverlayWidget(TSharedRef<SOverlay> InViewportOverlayWidget);
	void SetGameLayerManager(TSharedPtr< IGameLayerManager > LayerManager);

private:
	/** Overlay widget that contains widgets to draw on top of the game viewport */
	TWeakPtr<SOverlay> ViewportOverlayWidget;
	TWeakPtr<IGameLayerManager> GameLayerManagerPtr;
};

