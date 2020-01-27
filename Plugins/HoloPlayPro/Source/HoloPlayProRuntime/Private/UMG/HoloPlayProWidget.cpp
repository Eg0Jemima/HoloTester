#include "UMG/HoloPlayProWidget.h"

#include "IHoloPlayProRuntime.h"
#include "Slate/HoloPlayProViewportClient.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#define LOCTEXT_NAMESPACE "HoloPlayProWidget"

UHoloPlayProWidget* UHoloPlayProWidget::Create()
{
	return NewObject<UHoloPlayProWidget>();
}

bool UHoloPlayProWidget::AddToViewport(UUserWidget * Widget, int32 ZOrder)
{
	if (!FullScreenWidget.IsValid())
	{
		auto HoloPlayProViewportClient = IHoloPlayProRuntime::Get().GetHoloPlayProViewportClient();

		// First create and initialize the variable so that users calling this function twice don't
		// attempt to add the widget to the viewport again.
		TSharedRef<SConstraintCanvas> FullScreenCanvas = SNew(SConstraintCanvas);
		FullScreenWidget = FullScreenCanvas;

		TSharedRef<SWidget> UserSlateWidget = Widget->TakeWidget();

		FullScreenCanvas->AddSlot()
			.Offset(Widget->GetCachedWidget().IsValid() ? Widget->GetCachedWidget()->GetDesiredSize() : FVector2D())
			.Anchors(Widget->GetAnchorsInViewport())
			.Alignment(Widget->GetAlignmentInViewport())
			[
				UserSlateWidget
			];

		if (HoloPlayProViewportClient != nullptr)
		{
			return HoloPlayProViewportClient->AddViewportWidgetContent(FullScreenCanvas, ZOrder);
		}
	}
	else
	{
		FMessageLog("PIE").Warning(FText::Format(LOCTEXT("WidgetAlreadyOnScreen", "The widget '{0}' was already added to the screen."),
			FText::FromString(GetClass()->GetName())));
	}


	return false;
}

void UHoloPlayProWidget::RemoveFromViewport()
{
	if (FullScreenWidget.IsValid())
	{
		auto HoloPlayProViewportClient = IHoloPlayProRuntime::Get().GetHoloPlayProViewportClient();

		TSharedPtr<SWidget> WidgetHost = FullScreenWidget.Pin();

		TSharedRef<SWidget> WidgetHostRef = WidgetHost.ToSharedRef();

		HoloPlayProViewportClient->RemoveViewportWidgetContent(WidgetHostRef);
	}
	else
	{
	}
}


#undef LOCTEXT_NAMESPACE