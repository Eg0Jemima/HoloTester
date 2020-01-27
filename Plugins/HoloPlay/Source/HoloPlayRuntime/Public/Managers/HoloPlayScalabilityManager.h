#pragma once

#include "CoreMinimal.h"
#include "Managers/IHoloPlayManager.h"

/**
 * @class	HoloPlayScalabilityManager
 *
 * @brief	Manager for holo play command lines.
 */

class FHoloPlayScalabilityManager : public IHoloPlayManager
{
public:
	FHoloPlayScalabilityManager();
	virtual ~FHoloPlayScalabilityManager();
	/** IHoloPlayManager Interface */

	/**
	 * @fn	virtual bool FHoloPlayCommandLineManager::Init() override;
	 *
	 * @brief	Initializes this Manager class
	 *
	 * @returns	True if it Initializes successful , false if it fails.
	 */

	virtual bool Init() override;
	/** IHoloPlayManager Interface */

	virtual bool SetScalability(int32 Value);
};