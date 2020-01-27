// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IHoloPlayProEditor.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/Commands.h"

class FHoloPlayProEditorMenu;

/**
 * Display Cluster module implementation
 */
class FHoloPlayProEditor 
	: public  IHoloPlayProEditor
{
private:
	//////////////////////////////////////////////////////////////////////////////////////////////
	// IModuleInterface
	//////////////////////////////////////////////////////////////////////////////////////////////
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void AddEditorSettings();

private:
	TSharedPtr<FHoloPlayProEditorMenu> HoloPlayProEditorMenu;
};
