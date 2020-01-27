#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HoloPlayProLibrary.generated.h"

class UUserWidget;
class UHoloPlayProWidget;

UCLASS()
class HOLOPLAYPRORUNTIME_API UHoloPlayProLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HoloPlayPro")
	static UHoloPlayProWidget* CreateHoloPlayProWidget();
};
