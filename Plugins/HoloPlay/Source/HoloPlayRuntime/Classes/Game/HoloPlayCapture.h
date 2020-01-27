// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HoloPlaySettings.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HoloPlayCapture.generated.h"

class USpringArmComponent;
class USceneCaptureComponent2D;
class UHoloPlayCameraComponent;
class UTextureRenderTarget2D;

UCLASS(hidecategories = ("Actor", "LOD", "Cooking", "Rendering", "Replication", "Input"), meta = (AllowPrivateAccess = "true"))

/**
 * @class	AHoloPlayCapture
 *
 * @brief	The primary actor that handles visualisation and projection
 */

class HOLOPLAYRUNTIME_API AHoloPlayCapture : public AActor
{
	GENERATED_BODY()
	
public:	

	/**
	 * @fn	AHoloPlayCapture::AHoloPlayCapture();
	 *
	 * @brief	Sets default values for this actor's properties
	 */

	AHoloPlayCapture();

	FHoloPlayTilingQuality GetTilingValues() { return TilingValues; }

	UTexture2D* GetOverrideQuiltTexture2D() { return OverrideQuiltTexture2D; }

	USceneCaptureComponent2D* GetRenderCamera() { return RenderCamera; }

	/**
	 * @fn	virtual void AHoloPlayCapture::BeginPlay() override;
	 *
	 * @brief	Called when the game starts or when spawned
	 */

	virtual void BeginPlay() override;

	/**
	 * @fn	virtual void AHoloPlayCapture::EndPlay(const EEndPlayReason::Type EndPlayReason);
	 *
	 * @brief	called on ends a play
	 *
	 * @param	EndPlayReason	The end play reason.
	 */

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	/**
	 * @fn	virtual void AHoloPlayCapture::OnConstruction(const FTransform & Transform) override;
	 *
	 * @brief	Called on after reaading setting all variables from UObject loader
	 *
	 * @param	Transform	The transform.
	 */

	virtual void OnConstruction(const FTransform & Transform) override;

	/**
	 * @fn	void AHoloPlayCapture::HandleOffset(float HorizontalAngle, float VerticalAngle);
	 *
	 * @brief	Handles tiling offset
	 *
	 * @param	HorizontalAngle	The horizontal angle.
	 * @param	VerticalAngle  	The vertical angle.
	 */

	void HandleOffset(float HorizontalAngle, float VerticalAngle);

	/**
	 * @fn	float AHoloPlayCapture::AngleAtView(int View, int NumViews, float ViewCone);
	 *
	 * @brief	Calculate the tiling offset
	 *
	 * @param	View		The view.
	 * @param	NumViews	Number of views.
	 * @param	ViewCone	The view cone.
	 *
	 * @returns	A float.
	 */

	float AngleAtView(int View, int NumViews, float ViewCone);

	float GetCamDistance();

	/**
	 * @fn	FMatrix AHoloPlayCapture::GenerateProjectionMatrix(float FOV, float Near, float Far, float OffsetX, float OffsetY);
	 *
	 * @brief	Generates a projection matrix
	 *
	 * @param	FOV	   	The fov.
	 * @param	Near   	The near.
	 * @param	Far	   	The far.
	 * @param	OffsetX	The offset x coordinate.
	 * @param	OffsetY	The offset y coordinate.
	 *
	 * @returns	The projection matrix.
	 */

	FMatrix GenerateProjectionMatrix(float FOV, float Near, float Far, float OffsetX, float OffsetY);

	/**
	 * @fn	void AHoloPlayCapture::ResetCameras();
	 *
	 * @brief	Resets the tiling cameras after rendering to Quilt
	 */

	void ResetCameras();

	/**
	 * @fn	void AHoloPlayCapture::UpdateTillingProperties(EHoloPlayQualitySettings TilingQuailty);
	 *
	 * @brief	Called when Properties are Updated
	 *
	 * @param	TilingQuailty	The tiling quailty.
	 */

	void UpdateTillingProperties(EHoloPlayQualitySettings TilingQuailty);

	UFUNCTION(BlueprintCallable, Category = "Render")
	void RenderView(int ViewToRender, int NumViewsToRender);

#if WITH_EDITOR

	/**
	 * @fn	void AHoloPlayCapture::PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	 *
	 * @brief	Called then property was changed in Editor
	 *
	 * @param [in,out]	e	A FPropertyChangedEvent to process.
	 */

	void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

protected:
	//Customizable DrawFrustum Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Visual")
	FColor FrustumColor = FColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Visual", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
	float FrustumLineThickness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Visual")
	FColor MidPlaneFrustumColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Visual", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MidPlaneLineLength = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Visual", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
	float MidLineThickness = 0.0f;

	// Customizable Capture Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "2000.0"))
	float Size = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings")
	float NearClipFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings")
	bool bUseFarClipPlane = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings",  meta = (EditCondition = "bUseFarClipPlane"))
	float FarClipFactor = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings", meta = (ClampMin = "8.0", ClampMax = "90.0", UIMin = "8.0", UIMax = "90.0"))
	float FOV = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced")
	bool bUseCustomVerticalAngle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced", meta = (EditCondition = "bUseCustomVerticalAngle", ClampMin = "-60.0", ClampMax = "60.0", UIMin = "-60.0", UIMax = "60.0"))
	float CustomVerticalAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced")
	bool bUseCustomViewCone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced", meta = (EditCondition = "bUseCustomViewCone", ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float CustomViewCone = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced")
	bool EnableAdvancedRenderingFeatures = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureSettings|Advanced")
	bool bIsSimpleRender = true;

	// Customizable Quilting Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuiltSettings)
	UTexture2D* OverrideQuiltTexture2D = nullptr;

	// Customizable Tiling Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TilingSettings)
	EHoloPlayQualitySettings TilingQuality = EHoloPlayQualitySettings::Q_Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TilingSettings, meta = (ShowOnlyInnerProperties))
	FHoloPlayTilingQuality TilingValues;
	
	FHoloPlayTilingQuality CustomTilingValues;

	UPROPERTY(BlueprintReadWrite)
	FHoloPlayConfigValues VerticalAngle;

	UPROPERTY(BlueprintReadWrite)
	FHoloPlayConfigValues ViewCone;

	UPROPERTY(BlueprintReadWrite)
	float Aspect;

	UPROPERTY(BlueprintReadOnly, Category = "Projection Settings")
	float ResetCamerasView = 1.0f;

private:
	// Components Declarations
	USpringArmComponent* PreviewCameraSpringArm = nullptr;

	UHoloPlayCameraComponent* PreviewCamera = nullptr;

	USceneCaptureComponent2D* RenderCamera = nullptr;

	float NearClipPlane = 0.f;

	float FarClipPlane = 0.f;

	float NearClipMin = 1.0f;

private:
	void SetRenderingFeatures();

	/**
	 * @fn	void AHoloPlayCapture::ResetCam();
	 *
	 * @brief	Setup all cameras
	 */

	UFUNCTION(BlueprintCallable, Category = "HoloPlay")
	void ResetCam();

	void ResetCam(float VerticalAngle, float fov, bool resetOffset = true);

	/**
	 * @fn	void AHoloPlayCapture::ResizeTilingTexture(UTextureRenderTarget2D* TextureRenderTarget2D);
	 *
	 * @brief	Resize tiling texture ralated to tiling settings
	 *
	 * @param [in,out]	TextureRenderTarget2D	If non-null, the texture render target 2D.
	 */

	void ResizeTilingTexture(UTextureRenderTarget2D* TextureRenderTarget2D);
};
