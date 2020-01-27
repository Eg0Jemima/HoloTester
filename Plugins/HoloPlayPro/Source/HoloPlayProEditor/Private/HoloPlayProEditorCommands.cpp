#include "HoloPlayProEditorCommands.h"
#include "IHoloPlayProRuntime.h"

#include "SlateBasics.h"
#include "ISettingsModule.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "FHoloPlayProEditor"

void FHoloPlayProEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenControlScreenWindow, "OpenControlScreenWindow", "Open Control Screen Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CloseControlScreenWindow, "CloseControlScreenWindow", "Close Control Screen Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(HoloPlayProSettings, "HoloPlayProSettings", "Open Holo Play Pro Settings", EUserInterfaceActionType::Button, FInputChord());

	CommandActionList = MakeShareable(new FUICommandList);

	CommandActionList->MapAction(
		OpenControlScreenWindow,
		FExecuteAction::CreateRaw(this, &FHoloPlayProEditorCommands::OpenControlScreenWindowAction),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([this]() {
			return !IHoloPlayProRuntime::Get().IsExtendedScreenRunning();
		})
	);

	CommandActionList->MapAction(
		CloseControlScreenWindow,
		FExecuteAction::CreateRaw(this, &FHoloPlayProEditorCommands::CloseControlScreenWindowAction),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([this]() {
			return IHoloPlayProRuntime::Get().IsExtendedScreenRunning();
		})
	);

	CommandActionList->MapAction(
		HoloPlayProSettings,
		FExecuteAction::CreateLambda([this]()
			{
				// Put your "OnButtonClicked" stuff here
				FName ProjectSettingsTabName("ProjectSettings");
				if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
				{
					SettingsModule->ShowViewer("Project", "Plugins", "HoloPlayPro");
				}
			}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible()
	);
}

void FHoloPlayProEditorCommands::OpenControlScreenWindowAction()
{
	IHoloPlayProRuntime::Get().RunExtendedScreen();
}

void FHoloPlayProEditorCommands::CloseControlScreenWindowAction()
{
	IHoloPlayProRuntime::Get().StopExtendedScreen();
}

#undef LOCTEXT_NAMESPACE