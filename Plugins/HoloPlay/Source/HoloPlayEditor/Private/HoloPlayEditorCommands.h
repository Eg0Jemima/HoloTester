#pragma once

#include "HoloPlayEditorStyle.h"

#include "HoloPlaySettings.h"

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FUICommandList;
class FUICommandInfo;

/**
 * @class	FHoloPlayToolbarCommand
 *
 * @brief	Handles Unreal Engine holoplay editor commands
 */

class FHoloPlayToolbarCommand : public TCommands<FHoloPlayToolbarCommand>
{
public:
	FHoloPlayToolbarCommand();

	/**
	 * @fn	virtual void FHoloPlayToolbarCommand::RegisterCommands() override;
	 *
	 * @brief	Registers the commands for Unreal Engine
	 */

	virtual void RegisterCommands() override;

private:
	static void RepeatLastPlay_Clicked();
	static bool RepeatLastPlay_CanExecute();

	static void PlayInHoloPlayWindow_Clicked();
	static void CloseHoloPlayWindow_Clicked();
	static void OpenHoloPlaySettings_Clicked();
	static void PlayInMainViewport_Clicked();

	static bool PlayInModeIsChecked(EHoloPlayModeType PlayMode);

	static void OnTogglePlayInQuiltMode();
	static bool OnIsPlayInQuiltModeEnabled();

	static void OnToggleLockInMainViewport();
	static bool OnIsLockInMainViewport();	

	static void OnToggleAutoPlacementInHoloPlay();
	static bool IsAutoPlacementInHoloPlay();

	static bool IsPlaying();
	static bool IsNotPlaying();

	static bool CanExecutePlayInMainViewport();
	static bool CanExecutePlayInHoloPlayWindow();
	static bool CanExecuteCloseInHoloPlayWindow();


public:
	static int32 GetCurrentHoloPlayDisplayIndex();
	static void SetCurrentHoloPlayDisplayIndex(int32 Index, ETextCommit::Type CommitInfo);

	static int32 GetCurrentHoloPlayScalability();
	static void SetCurrentHoloPlayScalability(int32 Value, ETextCommit::Type CommitInfo);

	static void SetLastExecutedPlayMode(EHoloPlayModeType PlayMode);
	static const TSharedRef <FUICommandInfo> GetLastPlaySessionCommand();

public:
	TSharedPtr<FUICommandInfo>		RepeatLastPlay;
	TSharedPtr<FUICommandInfo>		PlayInHoloPlayWindow;
	TSharedPtr<FUICommandInfo>		CloseHoloPlayWindow;
	TSharedPtr<FUICommandInfo>		OpenHoloPlaySettings;
	TSharedPtr<FUICommandInfo>		PlayInMainViewport;
	TSharedPtr<FUICommandInfo>		PlayInQuiltMode;
	TSharedPtr<FUICommandInfo>		LockInMainViewport;
	TSharedPtr<FUICommandInfo>		AutoPlacementInHoloPlay;
	TSharedPtr<FUICommandList>		CommandActionList;
};