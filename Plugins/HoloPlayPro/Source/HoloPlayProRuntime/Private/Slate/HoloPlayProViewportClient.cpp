#include "Slate/HoloPlayProViewportClient.h"
#include "CanvasTypes.h"
#include "Engine.h"
#include "Slate/SGameLayerManager.h"
#include "Engine/LocalPlayer.h"

UHoloPlayProViewportClient::UHoloPlayProViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UHoloPlayProViewportClient::~UHoloPlayProViewportClient()
{
}

void UHoloPlayProViewportClient::Init()
{

}

void UHoloPlayProViewportClient::Tick(float DeltaTime)
{
}

bool UHoloPlayProViewportClient::AddViewportWidgetContent(TSharedRef<SWidget> ViewportContent, const int32 ZOrder)
{
	TSharedPtr< SOverlay > PinnedViewportOverlayWidget(ViewportOverlayWidget.Pin());
	if (ensure(PinnedViewportOverlayWidget.IsValid()))
	{
		// NOTE: Returns FSimpleSlot but we're ignoring here.  Could be used for alignment though.
		PinnedViewportOverlayWidget->AddSlot(ZOrder)
			[
				ViewportContent
			];

		return true;
	}

	return false;
}

void UHoloPlayProViewportClient::RemoveViewportWidgetContent(TSharedRef<SWidget> ViewportContent)
{
	TSharedPtr< SOverlay > PinnedViewportOverlayWidget(ViewportOverlayWidget.Pin());
	if (PinnedViewportOverlayWidget.IsValid())
	{
		PinnedViewportOverlayWidget->RemoveSlot(ViewportContent);
	}
}

void UHoloPlayProViewportClient::AddViewportWidgetForPlayer(ULocalPlayer * Player, TSharedRef<SWidget> ViewportContent, const int32 ZOrder)
{
	TSharedPtr< IGameLayerManager > GameLayerManager(GameLayerManagerPtr.Pin());
	if (GameLayerManager.IsValid())
	{
		GameLayerManager->AddWidgetForPlayer(Player, ViewportContent, ZOrder);
	}
}

void UHoloPlayProViewportClient::RemoveViewportWidgetForPlayer(ULocalPlayer * Player, TSharedRef<SWidget> ViewportContent)
{
	TSharedPtr< IGameLayerManager > GameLayerManager(GameLayerManagerPtr.Pin());
	if (GameLayerManager.IsValid())
	{
		GameLayerManager->RemoveWidgetForPlayer(Player, ViewportContent);
	}
}

void UHoloPlayProViewportClient::RemoveAllViewportWidgets()
{
	TSharedPtr< SOverlay > PinnedViewportOverlayWidget(ViewportOverlayWidget.Pin());
	if (PinnedViewportOverlayWidget.IsValid())
	{
		PinnedViewportOverlayWidget->ClearChildren();
	}

	TSharedPtr< IGameLayerManager > GameLayerManager(GameLayerManagerPtr.Pin());
	if (GameLayerManager.IsValid())
	{
		GameLayerManager->ClearWidgets();
	}
}

void UHoloPlayProViewportClient::SetViewportOverlayWidget(TSharedRef<SOverlay> InViewportOverlayWidget)
{
	ViewportOverlayWidget = InViewportOverlayWidget;
}

void UHoloPlayProViewportClient::SetGameLayerManager(TSharedPtr<IGameLayerManager> LayerManager)
{
	GameLayerManagerPtr = LayerManager;
}

void UHoloPlayProViewportClient::Draw(FViewport * InViewport, FCanvas * InCanvas)
{
	InCanvas->Clear(FLinearColor::Black);
}

UWorld * UHoloPlayProViewportClient::GetWorld() const
{
	return GWorld;
}

bool UHoloPlayProViewportClient::InputKey(FViewport * Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	if (GEngine->GameViewport != nullptr)
	{
		ULocalPlayer* FirstLocalPlayerFromController = GEngine->GameViewport->GetWorld()->GetFirstLocalPlayerFromController();

		if (FirstLocalPlayerFromController && FirstLocalPlayerFromController->PlayerController)
		{
			FirstLocalPlayerFromController->PlayerController->InputKey(Key, Event, AmountDepressed, bGamepad);
		}
	}

	return false;
}

bool UHoloPlayProViewportClient::InputAxis(FViewport * Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (GEngine->GameViewport != nullptr)
	{
		ULocalPlayer* FirstLocalPlayerFromController = GEngine->GameViewport->GetWorld()->GetFirstLocalPlayerFromController();

		if (FirstLocalPlayerFromController && FirstLocalPlayerFromController->PlayerController)
		{
			FirstLocalPlayerFromController->PlayerController->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
		}
	}

	return false;
}

bool UHoloPlayProViewportClient::InputTouch(FViewport * Viewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D & TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex)
{
	return false;
}

bool UHoloPlayProViewportClient::InputGesture(FViewport * Viewport, EGestureEvent GestureType, const FVector2D & GestureDelta, bool bIsDirectionInvertedFromDevice)
{
	return false;
}

bool UHoloPlayProViewportClient::InputMotion(FViewport * Viewport, int32 ControllerId, const FVector & Tilt, const FVector & RotationRate, const FVector & Gravity, const FVector & Acceleration)
{
	return false;
}
