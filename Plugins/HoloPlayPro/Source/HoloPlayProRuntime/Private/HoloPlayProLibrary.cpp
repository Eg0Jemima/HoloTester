#include "HoloPlayProLibrary.h"
#include "IHoloPlayProRuntime.h"
#include "UMG/HoloPlayProWidget.h"

UHoloPlayProWidget * UHoloPlayProLibrary::CreateHoloPlayProWidget()
{
	return UHoloPlayProWidget::Create();
}
