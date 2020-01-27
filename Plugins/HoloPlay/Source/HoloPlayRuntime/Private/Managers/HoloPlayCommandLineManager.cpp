#include "Managers/HoloPlayCommandLineManager.h"
#include "Managers/HoloPlayScalabilityManager.h"
#include "HoloPlaySettings.h"
#include "Misc/HoloPlayLog.h"
#include "IHoloPlayRuntime.h"

#include "Runtime/Core/Public/Misc/CommandLine.h"


FHoloPlayCommandLineManager::FHoloPlayCommandLineManager()
{
}

FHoloPlayCommandLineManager::~FHoloPlayCommandLineManager()
{
}

bool FHoloPlayCommandLineManager::Init()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	uint32 WindowType = 0;
	uint32 LastExecutedPlayModeType = 0;
	uint32 LockInMainViewport = 0;
	uint32 AutoPlacementInHoloPlay = 0;
	uint32 ScreenIndex = 0;
	uint32 Scalability = 0;

	int ShouldSave = 0;

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_windowtype="), WindowType))
	{
		HoloPlaySettings->HoloPlayWindowSettings.WindowType = static_cast<EHoloPlayWindowType>(WindowType);
		ShouldSave++;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_playmodetype="), LastExecutedPlayModeType))
	{
		HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType = static_cast<EHoloPlayModeType>(LastExecutedPlayModeType);
		ShouldSave++;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_lockinmainviewport="), LockInMainViewport))
	{
		HoloPlaySettings->HoloPlayWindowSettings.bLockInMainViewport = !!LockInMainViewport;
		ShouldSave++;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_autoplaycment="), AutoPlacementInHoloPlay))
	{
		HoloPlaySettings->HoloPlayWindowSettings.bAutoPlacementInHoloPlay = !!AutoPlacementInHoloPlay;
		ShouldSave++;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_screenindex="), ScreenIndex))
	{
		HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex = ScreenIndex;
		ShouldSave++;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("hp_scalability="), Scalability))
	{
		HoloPlaySettings->Scalability = Scalability;
		ShouldSave++;

		auto HoloPlayScalabilityManager = GHoloPlayRuntime->GetHoloPlayScalabilityManager();
		if (HoloPlayScalabilityManager.IsValid())
		{
			HoloPlayScalabilityManager->SetScalability(Scalability);
		}
	}

	if (ShouldSave > 0)
	{
		HoloPlaySettings->HoloPlaySave();
	}

	return true;
}
