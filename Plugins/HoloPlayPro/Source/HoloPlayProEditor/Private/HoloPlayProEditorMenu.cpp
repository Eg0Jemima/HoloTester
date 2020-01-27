#include "HoloPlayProEditorMenu.h"
#include "HoloPlayProEditorCommands.h"

#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "HoloPlayProEditorMenu"

FHoloPlayProEditorMenu::FHoloPlayProEditorMenu()
{
	ExtendHoloPlayMenu();
}

FHoloPlayProEditorMenu::~FHoloPlayProEditorMenu()
{
	if (UObjectInitialized() && MenuExtender.IsValid() && !GIsRequestingExit)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		if (LevelEditorModule.GetToolBarExtensibilityManager().IsValid())
		{
			LevelEditorModule.GetToolBarExtensibilityManager()->RemoveExtender(MenuExtender);
		}
	}
}

void FHoloPlayProEditorMenu::ExtendHoloPlayMenu()
{
	MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension(
		"HoloPlayScalability",
		EExtensionHook::After,
		FHoloPlayProEditorCommands::Get().CommandActionList,
		FMenuExtensionDelegate::CreateRaw(this, &FHoloPlayProEditorMenu::CreateMenu));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FHoloPlayProEditorMenu::CreateMenu(FMenuBuilder & MenuBuilder)
{
	MenuBuilder.BeginSection("HoloPlayProSection", LOCTEXT("HoloPlayProSection", "HoloPlayPro"));
	{
		MenuBuilder.AddMenuEntry(FHoloPlayProEditorCommands::Get().OpenControlScreenWindow);
		MenuBuilder.AddMenuEntry(FHoloPlayProEditorCommands::Get().CloseControlScreenWindow);
		MenuBuilder.AddMenuEntry(FHoloPlayProEditorCommands::Get().HoloPlayProSettings);
	}
	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE
