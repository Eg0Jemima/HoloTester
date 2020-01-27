#include "HoloPlayEditorCommands.h"

#include "IHoloPlayRuntime.h"
#include "Managers/HoloPlayDisplayManager.h"
#include "Managers/HoloPlayScalabilityManager.h"

#include "SlateBasics.h"
#include "ISettingsModule.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "FHoloPlayEditorModule"

FHoloPlayToolbarCommand::FHoloPlayToolbarCommand()
	: TCommands<FHoloPlayToolbarCommand>(
		"HoloPlay", // Context name for fast lookup
		NSLOCTEXT("HoloPlayToolbarCommands", "HoloPlayToolbar Commands", "HoloPlayToolbar Commands"), // Localized context name for displaying
		NAME_None,
		FHoloPlayEditorStyle::GetStyleSetName() // Icon Style Set
		)
{
}

void FHoloPlayToolbarCommand::SetLastExecutedPlayMode(EHoloPlayModeType PlayMode)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType = PlayMode;

	HoloPlaySettings->HoloPlaySave();
}

const TSharedRef<FUICommandInfo> FHoloPlayToolbarCommand::GetLastPlaySessionCommand()
{
	const UHoloPlaySettings* HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	const FHoloPlayToolbarCommand& Commands = FHoloPlayToolbarCommand::Get();
	TSharedRef < FUICommandInfo > Command = Commands.PlayInHoloPlayWindow.ToSharedRef();

	switch (HoloPlaySettings->HoloPlayWindowSettings.LastExecutedPlayModeType)
	{
	case EHoloPlayModeType::PlayMode_InMainViewport:
		Command = Commands.PlayInMainViewport.ToSharedRef();
		break;

	case EHoloPlayModeType::PlayMode_InSeparateWindow:
		Command = Commands.PlayInHoloPlayWindow.ToSharedRef();
		break;
	}

	return Command;
}

void FHoloPlayToolbarCommand::RegisterCommands()
{
	UI_COMMAND(RepeatLastPlay, "HoloPlay", "Repeat Last HoloPlay", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PlayInHoloPlayWindow, "PlayInHoloPlayWindow", "Open HoloPlay Window (Lock must be unchecked)", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(CloseHoloPlayWindow, "CloseHoloPlayWindow", "Close HoloPlay Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenHoloPlaySettings, "OpenHoloPlaySettings", "Open HoloPlay Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PlayInMainViewport, "PlayInMainViewport", "Play In Main Viewport (Game must be running)", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(PlayInQuiltMode, "Play in Quilt Mode", "If checked, quilt is rendered to viewport without lenticular shader", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(LockInMainViewport, "Lock in main viewport", "If checked, all play options will be locked. Rendering will be in main viewport", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(AutoPlacementInHoloPlay, "Auto Placement In HoloPlay", "If checked, HoloPlay screen will automatically be placed in Looking Glass display", EUserInterfaceActionType::ToggleButton, FInputChord());

	CommandActionList = MakeShareable(new FUICommandList);

	CommandActionList->MapAction(
		RepeatLastPlay,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::RepeatLastPlay_Clicked),
		FCanExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::RepeatLastPlay_CanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateStatic(&FHoloPlayToolbarCommand::IsNotPlaying)
	);

	CommandActionList->MapAction(
		PlayInHoloPlayWindow,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::PlayInHoloPlayWindow_Clicked),
		FCanExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::CanExecutePlayInHoloPlayWindow),
		FIsActionChecked::CreateStatic(&FHoloPlayToolbarCommand::PlayInModeIsChecked, EHoloPlayModeType::PlayMode_InSeparateWindow),
		FIsActionButtonVisible::CreateStatic(&FHoloPlayToolbarCommand::IsNotPlaying)
	);

	CommandActionList->MapAction(
		CloseHoloPlayWindow,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::CloseHoloPlayWindow_Clicked),
		FCanExecuteAction::CreateLambda(&FHoloPlayToolbarCommand::CanExecuteCloseInHoloPlayWindow),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateStatic(&FHoloPlayToolbarCommand::IsPlaying)
	);

	CommandActionList->MapAction(
		PlayInMainViewport,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::PlayInMainViewport_Clicked),
		FCanExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::CanExecutePlayInMainViewport),
		FIsActionChecked::CreateStatic(&FHoloPlayToolbarCommand::PlayInModeIsChecked, EHoloPlayModeType::PlayMode_InMainViewport),
		FIsActionButtonVisible::CreateStatic(&FHoloPlayToolbarCommand::IsNotPlaying)
	);

	CommandActionList->MapAction(
		OpenHoloPlaySettings,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::OpenHoloPlaySettings_Clicked),
		FCanExecuteAction()
	);

	CommandActionList->MapAction(
		PlayInQuiltMode,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::OnTogglePlayInQuiltMode),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FHoloPlayToolbarCommand::OnIsPlayInQuiltModeEnabled)
	);

	CommandActionList->MapAction(
		LockInMainViewport,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::OnToggleLockInMainViewport),
		FCanExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::IsNotPlaying),
		FIsActionChecked::CreateStatic(&FHoloPlayToolbarCommand::OnIsLockInMainViewport)
	);

	CommandActionList->MapAction(
		AutoPlacementInHoloPlay,
		FExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::OnToggleAutoPlacementInHoloPlay),
		FCanExecuteAction::CreateStatic(&FHoloPlayToolbarCommand::IsNotPlaying),
		FIsActionChecked::CreateStatic(&FHoloPlayToolbarCommand::IsAutoPlacementInHoloPlay)
	);
}

void FHoloPlayToolbarCommand::RepeatLastPlay_Clicked()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	// Grab the play command and execute it
	TSharedRef<FUICommandInfo> LastCommand = GetLastPlaySessionCommand();

	FHoloPlayToolbarCommand::Get().CommandActionList->ExecuteAction(LastCommand);
}

bool FHoloPlayToolbarCommand::RepeatLastPlay_CanExecute()
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	return FHoloPlayToolbarCommand::Get().CommandActionList->CanExecuteAction(GetLastPlaySessionCommand()) && 
		!HoloPlaySettings->HoloPlayWindowSettings.bLockInMainViewport;
}

void FHoloPlayToolbarCommand::PlayInHoloPlayWindow_Clicked()
{
	auto HoloPlayModeType = EHoloPlayModeType::PlayMode_InSeparateWindow;
	SetLastExecutedPlayMode(HoloPlayModeType);
	IHoloPlayRuntime::Get().StartPlayer(HoloPlayModeType);
}

void FHoloPlayToolbarCommand::CloseHoloPlayWindow_Clicked()
{
	IHoloPlayRuntime::Get().StopPlayer();
}

void FHoloPlayToolbarCommand::OpenHoloPlaySettings_Clicked()
{
	// Put your "OnButtonClicked" stuff here
	FName ProjectSettingsTabName("ProjectSettings");
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Plugins", "HoloPlay");
	}
}

void FHoloPlayToolbarCommand::PlayInMainViewport_Clicked()
{
	auto HoloPlayModeType = EHoloPlayModeType::PlayMode_InMainViewport;
	SetLastExecutedPlayMode(HoloPlayModeType);
	IHoloPlayRuntime::Get().StartPlayer(HoloPlayModeType);
}

bool FHoloPlayToolbarCommand::PlayInModeIsChecked(EHoloPlayModeType PlayMode)
{
	bool result = (PlayMode == GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.LastExecutedPlayModeType);

	return result;
}

void FHoloPlayToolbarCommand::OnTogglePlayInQuiltMode()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	HoloPlaySettings->HoloPlayShaderSettings.QuiltMode ^= 1;

	HoloPlaySettings->HoloPlaySave();
}

bool FHoloPlayToolbarCommand::OnIsPlayInQuiltModeEnabled()
{
	return !!(GetDefault<UHoloPlaySettings>()->HoloPlayShaderSettings.QuiltMode);
}

void FHoloPlayToolbarCommand::OnToggleLockInMainViewport()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	HoloPlaySettings->HoloPlayWindowSettings.bLockInMainViewport ^= true;

	HoloPlaySettings->HoloPlaySave();
}

bool FHoloPlayToolbarCommand::OnIsLockInMainViewport()
{
	return GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport;
}

void FHoloPlayToolbarCommand::OnToggleAutoPlacementInHoloPlay()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	HoloPlaySettings->HoloPlayWindowSettings.bAutoPlacementInHoloPlay ^= true;

	HoloPlaySettings->HoloPlaySave();
}

bool FHoloPlayToolbarCommand::IsAutoPlacementInHoloPlay()
{
	return GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bAutoPlacementInHoloPlay;
}

bool FHoloPlayToolbarCommand::IsPlaying()
{
	return IHoloPlayRuntime::Get().IsPlaying();
}

bool FHoloPlayToolbarCommand::IsNotPlaying()
{
	return !IHoloPlayRuntime::Get().IsPlaying();
}

bool FHoloPlayToolbarCommand::CanExecutePlayInMainViewport()
{
	return IsNotPlaying() && GEngine->GameViewport != nullptr && !GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport;
}

bool FHoloPlayToolbarCommand::CanExecutePlayInHoloPlayWindow()
{
	return IsNotPlaying() && !GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport;
}

bool FHoloPlayToolbarCommand::CanExecuteCloseInHoloPlayWindow()
{
	return IsPlaying() && !GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport;
}

int32 FHoloPlayToolbarCommand::GetCurrentHoloPlayDisplayIndex()
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	return HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex;
}

void FHoloPlayToolbarCommand::SetCurrentHoloPlayDisplayIndex(int32 Index, ETextCommit::Type CommitInfo)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	auto HoloPlayDisplayManager = IHoloPlayRuntime::Get().GetHoloPlayDisplayManager();
	if (HoloPlayDisplayManager.IsValid())
	{
		auto Metrics = HoloPlayDisplayManager->GetHoloPlayDisplayMetrics();

		if(Metrics.LKGcount == 0)
		{
			return;
		}

		if (Index < Metrics.LKGcount)
		{
			HoloPlaySettings->HoloPlayWindowSettings.ScreenIndex = Index;

			HoloPlaySettings->HoloPlaySave();
		}
	}
}

int32 FHoloPlayToolbarCommand::GetCurrentHoloPlayScalability()
{
	auto HoloPlaySettings = GetDefault<UHoloPlaySettings>();

	return HoloPlaySettings->Scalability;
}

void FHoloPlayToolbarCommand::SetCurrentHoloPlayScalability(int32 Value, ETextCommit::Type CommitInfo)
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();

	auto HoloPlayScalabilityManager = IHoloPlayRuntime::Get().GetHoloPlayScalabilityManager();
	if (HoloPlayScalabilityManager.IsValid())
	{
		HoloPlayScalabilityManager->SetScalability(Value);
	}

	HoloPlaySettings->Scalability = Value;
	HoloPlaySettings->HoloPlaySave();
}

#undef LOCTEXT_NAMESPACE
