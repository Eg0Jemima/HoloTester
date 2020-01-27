#include "HoloPlayProEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateTypes.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleSet->RootToContentDir( RelativePath, TEXT( ".png" ) ), __VA_ARGS__ )
#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FHoloPlayProEditorStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleSet->RootToContentDir( RelativePath, TEXT( ".png" ) ), __VA_ARGS__ )
#define TTF_CORE_FONT( RelativePath, ... ) FSlateFontInfo( StyleSet->RootToCoreContentDir( RelativePath, TEXT( ".ttf" ) ), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FHoloPlayProEditorStyle::StyleSet = nullptr;

FString FHoloPlayProEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("HoloPlayPro"))->GetContentDir() / TEXT("Slate");
	return (ContentDir / RelativePath) + Extension;
}

void FHoloPlayProEditorStyle::Initialize()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShared<FSlateStyleSet>(GetStyleSetName());

	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon512x512(512.0f, 512.0f);
	const FLinearColor DimBackground = FLinearColor(FColor(64, 64, 64));
	const FLinearColor DimBackgroundHover = FLinearColor(FColor(80, 80, 80));
	const FLinearColor LightBackground = FLinearColor(FColor(128, 128, 128));
	const FLinearColor HighlightedBackground = FLinearColor(FColor(255, 192, 0));

	StyleSet->Set("HoloPlayPro.OpenControlScreenWindow", new IMAGE_PLUGIN_BRUSH("Icons/OpenControlScreenWindow", Icon40x40));
	StyleSet->Set("HoloPlayPro.OpenControlScreenWindow.Small", new IMAGE_PLUGIN_BRUSH("Icons/OpenControlScreenWindow", Icon20x20));
	StyleSet->Set("HoloPlayPro.OpenControlScreenWindow.Selected", new IMAGE_PLUGIN_BRUSH("Icons/OpenControlScreenWindow", Icon40x40));
	StyleSet->Set("HoloPlayPro.OpenControlScreenWindow.Selected.Small", new IMAGE_PLUGIN_BRUSH("Icons/OpenControlScreenWindow", Icon20x20));

	StyleSet->Set("HoloPlayPro.HoloPlayProSettings", new IMAGE_PLUGIN_BRUSH("Icons/icon_game_settings_40x", Icon40x40));
	StyleSet->Set("HoloPlayPro.HoloPlayProSettings.Small", new IMAGE_PLUGIN_BRUSH("Icons/icon_game_settings_40x", Icon20x20));
	StyleSet->Set("HoloPlayPro.HoloPlayProSettings.Selected", new IMAGE_PLUGIN_BRUSH("Icons/icon_game_settings_40x", Icon40x40));
	StyleSet->Set("HoloPlayPro.HoloPlayProSettings.Selected.Small", new IMAGE_PLUGIN_BRUSH("Icons/icon_game_settings_40x", Icon20x20));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}


void FHoloPlayProEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}


FName FHoloPlayProEditorStyle::GetStyleSetName()
{
	static FName StyleName("HoloPlayProEditorStyle");
	return StyleName;
}
