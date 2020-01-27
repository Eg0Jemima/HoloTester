#include "Managers/HoloPlayDisplayManager.h"

#include "HoloPlaySettings.h"

#include "Misc/HoloPlayLog.h"

#include "SlateApplication.h"

#include "holoplay.h"

FHoloPlayDisplayMetrics::FCalibration::FCalibration(int i)
{
	Model = GetModel(i);
	ScreenWidth = GetScreenWidth(i);
	ScreenHeight = GetScreenHeight(i);
	DPI = GetDPI(i);
	ViewCone = GetViewCone(i);
	Aspect = GetAspect(i);
	Pitch = GetPitch(i);
	Slope = GetSlope(i);
	Center = GetCenter(i);

	char Buffer[120] = { 0 };

	GetSerial(i, Buffer);
	Serial = FString(ANSI_TO_TCHAR(Buffer));

	GetLKGName(i, Buffer);
	LKGName = FString(ANSI_TO_TCHAR(Buffer));
}


FHoloPlayDisplayMetrics::FDisplay::FDisplay(int i)
{
	char Buffer[120] = { 0 };

	LKGcalIndex = GetLKGcalIndex(i);
	LKGunityIndex = GetLKGunityIndex(i);
	LKGxpos = GetLKGxpos(i);
	LKGypos = GetLKGypos(i);

	GetLKGdisplayName(i, Buffer);
	LKGdisplayName = FString(ANSI_TO_TCHAR(Buffer));
}

FHoloPlayDisplayMetrics::FLoader::FLoader()
{
	LoadCalibrationResults = PopulateLKGDisplays();
	CalibrationFound = (LoadCalibrationResults & 3) != 0;
	DisplayFound = (LoadCalibrationResults & 4) != 0;
}

FHoloPlayDisplayManager::FHoloPlayDisplayManager()
{
}

FHoloPlayDisplayManager::~FHoloPlayDisplayManager()
{
}

bool FHoloPlayDisplayManager::Init()
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}

	// Get OS display settings
	auto PlatformApplication = FSlateApplication::Get().GetPlatformApplication();
	if (!PlatformApplication->OnDisplayMetricsChanged().IsBoundToObject(this))
	{
		PlatformApplication->OnDisplayMetricsChanged().AddRaw(this, &FHoloPlayDisplayManager::OnDisplayMetricsChanged);
	}
	PlatformApplication->GetInitialDisplayMetrics(DisplayMetrics);

	bool bIsSuccsessful = SetHoloPlayDisplaySettings(HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex);
	if (bIsSuccsessful)
	{
		ApplyCalibrations();
	}
	PrintDebugInfo();

	return true;
}

void FHoloPlayDisplayManager::Release()
{
}

bool FHoloPlayDisplayManager::OnStartPlayer(EHoloPlayModeType HoloPlayModeType)
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	// Update Holoplay Display Settings
	bool bIsSuccsessful = SetHoloPlayDisplaySettings(HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex);
	if (bIsSuccsessful)
	{
		ApplyCalibrations();
	}
	PrintDebugInfo();


	return true;
}

void FHoloPlayDisplayManager::OnStopPlayer()
{
}

bool FHoloPlayDisplayManager::SetHoloPlayDisplaySettings(int32 ScreenIndex)
{
	FHoloPlayDisplayMetrics::FLoader Loader;

	UE_LOG(HoloPlayLogManagers, Verbose, TEXT("LoadCalibrationResults %d, CalibrationFound %d, DisplayFound %d"), Loader.LoadCalibrationResults, Loader.CalibrationFound, Loader.DisplayFound);

	// Continue parsing only if calibratioin and display found
	if (Loader.CalibrationFound && Loader.DisplayFound)
	{
		HoloPlayDisplayMetrics.CalibrationCount = CalibrationCount();
		HoloPlayDisplayMetrics.LKGcount = GetLKGcount();

		// Get First all teh time
		int TargetLookingGlass = 0;
		Calibration = FHoloPlayDisplayMetrics::FCalibration(GetLKGcalIndex(0));
		for (int32 i = 0; i < HoloPlayDisplayMetrics.LKGcount; i++)
		{
			UE_LOG(HoloPlayLogManagers, Verbose, TEXT("ScreenIndex %d, GetLKGunityIndex(i) %d, GetLKGcalIndex(i) %d, i %d"), 
				ScreenIndex, GetLKGunityIndex(i), GetLKGcalIndex(i), i);

			if (ScreenIndex == i)
			{
				Calibration = FHoloPlayDisplayMetrics::FCalibration(GetLKGcalIndex(i));
				TargetLookingGlass = i;
			}
		}

		Display = FHoloPlayDisplayMetrics::FDisplay(TargetLookingGlass);

		return true;
	}

	return false;
}

bool FHoloPlayDisplayManager::ApplyCalibrations()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	HoloPlaySettings->HoloPlayShaderSettings.SetDefaults
	(
		Calibration.Pitch,
		Calibration.Slope,
		Calibration.Center,
		Calibration.ViewCone,
		Calibration.DPI,
		Calibration.Aspect,
		Calibration.ScreenWidth,
		Calibration.ScreenHeight,
		Calibration.Model
	);

	HoloPlaySettings->HoloPlaySave();

	return true;
}


void FHoloPlayDisplayManager::PrintDebugInfo()
{
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> DisplayMetrics --------------"));
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> HoloPlayDisplayMetrics --------------"));
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> CalibrationCount: %d"), HoloPlayDisplayMetrics.CalibrationCount);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGcount: %d"), HoloPlayDisplayMetrics.LKGcount);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> ---------------------------------------------------- <<"));

	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Display.Model: %d"), Calibration.Model);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> ScreenWidth: %d"), Calibration.ScreenWidth);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> ScreenHeight: %d"), Calibration.ScreenHeight);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> DPI: %f"), Calibration.DPI);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> ViewCone: %f"), Calibration.ViewCone);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Aspect: %f"), Calibration.Aspect);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Pitch: %f"), Calibration.Pitch);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Slope: %f"), Calibration.Slope);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Center: %f"), Calibration.Center);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> Serial: %s"), *Calibration.Serial);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGName: %s"), *Calibration.LKGName);

	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGcalIndex: %d"), Display.LKGcalIndex);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGunityIndex: %d"), Display.LKGunityIndex);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGxpos: %d"), Display.LKGxpos);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGypos: %d"), Display.LKGypos);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> LKGdisplayName: %s"), *Display.LKGdisplayName);
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT(">> ---------------------------------------------------- <<"));
}

void FHoloPlayDisplayManager::OnDisplayMetricsChanged(const FDisplayMetrics& InDisplayMetrics)
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	// Reload calibration only if amount of displays has been changed
	bool DisplaysNumChanged = false;
	if (InDisplayMetrics.MonitorInfo.Num() != DisplayMetrics.MonitorInfo.Num())
	{
		bool bIsSuccsessful = SetHoloPlayDisplaySettings(HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex);
		if (bIsSuccsessful)
		{
			ApplyCalibrations();
		}

		DisplaysNumChanged = true;
	}

	DisplayMetrics = InDisplayMetrics;

	if (DisplaysNumChanged)
	{
		PrintDebugInfo();
	}
}
