#include "Managers/HoloPlayScalabilityManager.h"
#include "HoloPlaySettings.h"
#include "Misc/HoloPlayLog.h"

#include "Runtime/Core/Public/Misc/CommandLine.h"

#include "Scalability.h"


FHoloPlayScalabilityManager::FHoloPlayScalabilityManager()
{
}

FHoloPlayScalabilityManager::~FHoloPlayScalabilityManager()
{
}

bool FHoloPlayScalabilityManager::Init()
{
	const UHoloPlaySettings* HoloPlaySettings = GetDefault<UHoloPlaySettings>();
	SetScalability(HoloPlaySettings->Scalability);

	return true;
}

bool FHoloPlayScalabilityManager::SetScalability(int32 Value)
{
	UE_LOG(HoloPlayLogManagers, Verbose, TEXT("SetScalability Value %d"), Value);

	Scalability::FQualityLevels QualityLevels;
	QualityLevels.SetFromSingleQualityLevel(Value);
	SetQualityLevels(QualityLevels);
	Scalability::SaveState(GIsEditor ? GEditorSettingsIni : GGameUserSettingsIni);

	return true;
}
