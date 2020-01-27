// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IHoloPlayProRuntime.h"
#include "SWindow.h"

class SHoloPlayProViewport;
class UHoloPlayProViewportClient;

/**
 * Display Cluster module implementation
 */
class FHoloPlayProRuntime :
	public  IHoloPlayProRuntime
{
public:
	FHoloPlayProRuntime();
	virtual ~FHoloPlayProRuntime();

private:
	//////////////////////////////////////////////////////////////////////////////////////////////
	// IModuleInterface
	//////////////////////////////////////////////////////////////////////////////////////////////
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** IHoloPlayProRuntime */
	virtual void RunExtendedScreen() override;
	virtual void StopExtendedScreen(bool bCloseWindow = true) override;
	virtual bool IsExtendedScreenRunning() override;

public:
	virtual UHoloPlayProViewportClient* GetHoloPlayProViewportClient() override;
	virtual TWeakPtr<SHoloPlayProViewport> GetSHoloPlayProViewport() override;

private:
	virtual void OnWindowClosed(const TSharedRef<SWindow>& Window);
	void OnPostEngineInit();

private:
	TSharedPtr<SWindow> ExtendedScreenWindow;
	TSharedPtr<SHoloPlayProViewport> HoloPlayProViewport;
	bool bIsExtendedScreenRunning;
	FOnWindowClosed OnWindowClosedDelegate;
};
