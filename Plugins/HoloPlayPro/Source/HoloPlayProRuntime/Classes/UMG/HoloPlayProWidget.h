#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"

#include "HoloPlayProWidget.generated.h"

class SWidget;
class UUserWidget;

UCLASS(BlueprintType)
class HOLOPLAYPRORUNTIME_API UHoloPlayProWidget : public UObject
{
	GENERATED_BODY()

public:
	static UHoloPlayProWidget* Create();

	UFUNCTION(BlueprintCallable, Category = "HoloPlayPro")
	virtual bool AddToViewport(UUserWidget* Widget, int32 ZOrder = 0);

	UFUNCTION(BlueprintCallable, Category = "HoloPlayPro")
	virtual void RemoveFromViewport();

private:
	TWeakPtr<SWidget> FullScreenWidget;
};