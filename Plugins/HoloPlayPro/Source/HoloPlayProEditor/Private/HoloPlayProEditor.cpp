#include "HoloPlayProEditor.h"
#include "HoloPlayProEditorStyle.h"
#include "HoloPlayProEditorCommands.h"
#include "HoloPlayProEditorMenu.h"

#include "HoloPlayProSettings.h"

#include "IHoloPlayEditor.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

#include "Editor.h"

#define LOCTEXT_NAMESPACE "FHoloPlayProEditor"

void FHoloPlayProEditor::StartupModule()
{
	FHoloPlayProEditorStyle::Initialize();
	FHoloPlayProEditorCommands::Register();

	// Add HoloPlayToolbar section
	HoloPlayProEditorMenu = MakeShareable(new FHoloPlayProEditorMenu);

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FHoloPlayProEditor::AddEditorSettings);
}

void FHoloPlayProEditor::ShutdownModule()
{
	FHoloPlayProEditorStyle::Shutdown();
	FHoloPlayProEditorCommands::Unregister();
	HoloPlayProEditorMenu.Reset();
}

void FHoloPlayProEditor::AddEditorSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "HoloPlayPro",
			LOCTEXT("HoloPlayProSettingsName", "HoloPlayPro Plugin"),
			LOCTEXT("HoloPlayProSettingsDescription", "Configure the HoloPlayPro plug-in."),
			GetMutableDefault<UHoloPlayProSettings>()
		);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FHoloPlayProEditor, HoloPlayProEditor)
