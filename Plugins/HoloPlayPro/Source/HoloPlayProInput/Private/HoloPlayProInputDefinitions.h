// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "InputCoreTypes.h"

#define GlobalGestureDoubleTapName "Global_Gesture_DoubleTap"
#define GlobalGestureDoubleTapFriendlyName "Global Gesture DoubleTap"

#define HoloPlayViewportGestureDoubleTapName "HoloPlayViewport_Gesture_DoubleTap"
#define HoloPlayViewportGestureDoubleTapFriendlyName "HoloPlay Viewport Gesture DoubleTap"

/*
* Keys struct
*/

struct FHoloPlayProInputKeys
{
	static const FKey GlobalGesture_DoubleTap;
	static const FKey HoloPlayViewportGesture_DoubleTap;
};
