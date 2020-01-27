#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

#include "HoloPlayProEditorStyle.h"

class FUICommandList;
class FUICommandInfo;

class FHoloPlayProEditorCommands : public TCommands<FHoloPlayProEditorCommands>
{
public:
	FHoloPlayProEditorCommands() : TCommands<FHoloPlayProEditorCommands>
		(
			"HoloPlayPro",
			NSLOCTEXT("HoloPlayPro", "HoloPlayPro", "HoloPlayPro"),
			NAME_None, // "MainFrame" // @todo Fix this crash
			FHoloPlayProEditorStyle::GetStyleSetName() // Icon Style Set
			)
	{}

	TSharedPtr<FUICommandInfo> OpenControlScreenWindow;
	TSharedPtr<FUICommandInfo> CloseControlScreenWindow;
	TSharedPtr<FUICommandInfo> HoloPlayProSettings;
	TSharedPtr<FUICommandList> CommandActionList;	


	virtual void RegisterCommands() override;

private:
	void OpenControlScreenWindowAction();
	void CloseControlScreenWindowAction();
};
