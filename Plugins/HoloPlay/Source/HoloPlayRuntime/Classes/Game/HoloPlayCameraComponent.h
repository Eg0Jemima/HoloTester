// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptInterface.h"
#include "Components/SceneComponent.h"
#include "Engine/BlendableInterface.h"
#include "Engine/Scene.h"
#include "Camera/CameraTypes.h"

#include "HoloPlayCameraComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UHoloPlayDrawFrustumComponent;


UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))

/**
 * @class	UHoloPlayCameraComponent
 *
 * @brief	Represents a camera viewpoint and settings, such as projection type, field of view,
 * 			and post-process overrides. The default behavior for an actor used as the camera view
 * 			target is to look for an attached camera component and use its location, rotation,
 * 			and settings.
 */

class HOLOPLAYRUNTIME_API UHoloPlayCameraComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()

public:

	/** The horizontal field of view (in degrees) in perspective mode (ignored in Orthographic mode) */
	float FieldOfView;

	/** The holoplay focus plane distance of the orthographic view (in world units) */
	float HoloPlayPlane;

	/** The Vertical Angle for Vertical Offset */
	float VertAngle;

	/** The near plane distance of the  view (in world units) */
	float NearClipPlane;

	/** The far plane distance of the  view (in world units) */
	float FarClipPlane;

	// Aspect Ratio (Width/Height)
	float AspectRatio;

	// MidPlane Line Length for Frustum Component
	float MidPlaneLineLength;

	// MidPlane Line Thickness for Frustum Component
	float MidLineThickness;

	// Frustum Line Thickness for Frustum Component
	float FrustumLineThickness;

	// The type of camera
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode;

protected:
	// The frustum component used to show visually where the camera field of view is
	UHoloPlayDrawFrustumComponent* DrawFrustum;

	UStaticMesh* CameraMesh;

	// The camera mesh to show visually where the camera is placed
	UStaticMeshComponent* ProxyMeshComponent;

public:
#if WITH_EDITOR

	// UActorComponent interface
	virtual void OnRegister() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void CheckForErrors() override;
	// End of UActorComponent interface

	// USceneComponent interface
	virtual bool GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut) override;
	// End of USceneComponent interface

	// UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface

	virtual void RefreshVisualRepresentation();

	void OverrideFrustumColor(FColor OverrideColor, FColor OverrideMidColor);

	void RestoreFrustumColor();
#endif
};

