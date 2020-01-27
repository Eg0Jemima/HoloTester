// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "HoloPlayEditor.h"

#include "HoloPlayEditorStyle.h"
#include "HoloPlayEditorCommands.h"
#include "HoloPlayToolbar.h"

#include "IHoloPlayRuntime.h"
#include "HoloPlaySettings.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

#include "Engine/GameViewportClient.h"
#include "Editor.h"
#include "Toolkits/AssetEditorToolkit.h"


#define LOCTEXT_NAMESPACE "FHoloPlayEditorModule"

void FHoloPlayEditorModule::StartupModule()
{
	// Initialize play button ui style
	FHoloPlayEditorStyle::Initialize();
	FHoloPlayEditorStyle::ReloadTextures();

	// Add HoloPlayToolbar section
	FHoloPlayToolbarCommand::Register();
	HoloPlayToolbar = MakeShareable(new FHoloPlayToolbar);

	// Add settings to project settings
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FHoloPlayEditorModule::AddEditorSettings);
	GetMutableDefault<UHoloPlaySettings>()->AddToRoot();

	UGameViewportClient::OnViewportCreated().AddRaw(this, &FHoloPlayEditorModule::OnPIEViewportStarted);
	FEditorDelegates::EndPIE.AddRaw(this, &FHoloPlayEditorModule::OnEndPIE);
}

void FHoloPlayEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FHoloPlayEditorStyle::Shutdown();

	// Release Toolbar
	HoloPlayToolbar.Reset();

	// Release editor settings
	RemoveEditorSettings();

	// Stop player if running
	IHoloPlayRuntime::Get().StopPlayer();
}

// Check MagicLeapHMD settings loading
void FHoloPlayEditorModule::AddEditorSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "HoloPlay",
			LOCTEXT("HoloPlaySettingsName", "HoloPlay Plugin"),
			LOCTEXT("HoloPlaySettingsDescription", "Configure the HoloPlay plug-in."),
			GetMutableDefault<UHoloPlaySettings>()
		);

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FHoloPlayEditorModule::OnSettingsSaved);
		}
	}
}

void FHoloPlayEditorModule::RemoveEditorSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "HoloPlay");
	}
}

bool FHoloPlayEditorModule::OnSettingsSaved()
{
	UHoloPlaySettings* HoloPlaySettings = GetMutableDefault<UHoloPlaySettings>();
	bool ResaveSettings = true;

	if (ResaveSettings)
	{
		HoloPlaySettings->HoloPlaySave();
	}

	return true;
}

void FHoloPlayEditorModule::OnPIEViewportStarted()
{
	if (GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport)
	{
		IHoloPlayRuntime::Get().StartPlayer(EHoloPlayModeType::PlayMode_InMainViewport);
	}
}

void FHoloPlayEditorModule::OnEndPIE(bool bIsSimulating)
{
	if (GetDefault<UHoloPlaySettings>()->HoloPlayWindowSettings.bLockInMainViewport)
	{
		IHoloPlayRuntime::Get().StopPlayer();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHoloPlayEditorModule, HoloPlayEditor)