#include "HoloPlaySettings.h"
#include "IHoloPlayRuntime.h"
#include "Managers/HoloPlayScalabilityManager.h"
#include "Misc/HoloPlayLog.h"

#include "Engine.h"

void FHoloPlayRenderingSettings::UpdateVsync()
{
	if (GEngine)
	{
		static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.VSync"));
		bool CVarbVsync = CVar->GetValueOnGameThread() != 0;
		if (CVarbVsync == bVsync)
		{
			return;
		}

		if (bVsync)
		{
			new(GEngine->DeferredCommands) FString(TEXT("r.vsync 1"));
		}
		else
		{
			new(GEngine->DeferredCommands) FString(TEXT("r.vsync 0"));
		}
	}
}

#if WITH_EDITOR
void UHoloPlaySettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		FName PropertyName(PropertyChangedEvent.Property->GetFName());

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UHoloPlaySettings, Scalability))
		{
			UE_LOG(HoloPlayLogSettings, Verbose, TEXT("PostEditChangeProperty"));

			auto HoloPlayScalabilityManager = GHoloPlayRuntime->GetHoloPlayScalabilityManager();
			if (HoloPlayScalabilityManager.IsValid())
			{
				HoloPlayScalabilityManager->SetScalability(Scalability);
			}
		}
	}
}
#endif

void UHoloPlaySettings::PostEngineInit() const
{
	for (auto Settings : HoloPlaySettings)
	{
		Settings->PostEngineInit();
	}
}

void UHoloPlaySettings::HoloPlayPreSave() const
{
	for (auto Settings : HoloPlaySettings)
	{
		Settings->PreSave();
	}
}
