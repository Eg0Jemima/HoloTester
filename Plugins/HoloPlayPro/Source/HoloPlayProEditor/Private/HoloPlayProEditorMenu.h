#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class FExtender;

class FHoloPlayProEditorMenu : public TSharedFromThis<FHoloPlayProEditorMenu>
{
public:
	FHoloPlayProEditorMenu();
	virtual ~FHoloPlayProEditorMenu();

private:
	void ExtendHoloPlayMenu();
	void CreateMenu(FMenuBuilder& MenuBuilder);

private:
	TSharedPtr<FExtender> MenuExtender;
};
