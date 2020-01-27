// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Game/HoloPlayCameraComponent.h"
#include "Game/HoloPlayDrawFrustumComponent.h"

#include "Misc/HoloPlayLog.h"

#include "UObject/ConstructorHelpers.h"
#include "EngineGlobals.h"

#include "Camera/CameraActor.h"
#include "Engine/Engine.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"

#include "Math/UnitConversion.h"
#include "Widgets/Input/NumericTypeInterface.h"

#define LOCTEXT_NAMESPACE "HoloPlayCameraComponent"


//////////////////////////////////////////////////////////////////////////
// UHoloPlayCameraComponent


UHoloPlayCameraComponent::UHoloPlayCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default initialisation
	FieldOfView = 0.f;
	HoloPlayPlane = 0.f;
	NearClipPlane = 0.f;
	FarClipPlane = 0.f;
	AspectRatio = 0.f;
	NearClipPlane = 0.f;
	VertAngle = 0.f;

	MidPlaneLineLength = 0.0f;
	MidLineThickness = 0.f;
	FrustumLineThickness = 0.f;

	ProjectionMode = ECameraProjectionMode::Perspective;

	DrawFrustum = nullptr;
	CameraMesh = nullptr;
	ProxyMeshComponent = nullptr;

#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> EditorCameraMesh(TEXT("/Engine/EditorMeshes/MatineeCam_SM"));
		CameraMesh = EditorCameraMesh.Object;
	}
#endif	// WITH_EDITOR
	bAutoActivate = true;
}

// Compile onlu for editor only
#if WITH_EDITOR

void UHoloPlayCameraComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (ProxyMeshComponent)
	{
		ProxyMeshComponent->DestroyComponent();
	}
	if (DrawFrustum)
	{
		DrawFrustum->DestroyComponent();
	}
}


void UHoloPlayCameraComponent::OnRegister()
{
	if (AActor* MyOwner = GetOwner())
	{
		if (ProxyMeshComponent == nullptr)
		{
			ProxyMeshComponent = NewObject<UStaticMeshComponent>(MyOwner, NAME_None, RF_Transactional | RF_TextExportTransient);
			ProxyMeshComponent->SetupAttachment(this);
			ProxyMeshComponent->SetIsVisualizationComponent(true);
			ProxyMeshComponent->SetStaticMesh(CameraMesh);
			ProxyMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			ProxyMeshComponent->bHiddenInGame = true;
			ProxyMeshComponent->CastShadow = false;
			ProxyMeshComponent->PostPhysicsComponentTick.bCanEverTick = false;
			ProxyMeshComponent->CreationMethod = CreationMethod;
			ProxyMeshComponent->RegisterComponentWithWorld(GetWorld());
		}

		if (DrawFrustum == nullptr)
		{
			DrawFrustum = NewObject<UHoloPlayDrawFrustumComponent>(MyOwner, NAME_None, RF_Transactional | RF_TextExportTransient);
			DrawFrustum->SetupAttachment(this);
			DrawFrustum->SetIsVisualizationComponent(true);
			DrawFrustum->CreationMethod = CreationMethod;
			DrawFrustum->RegisterComponentWithWorld(GetWorld());
		}
	}

	RefreshVisualRepresentation();

	Super::OnRegister();
}

void UHoloPlayCameraComponent::RefreshVisualRepresentation()
{
	if (DrawFrustum != nullptr)
	{
		if (ProjectionMode == ECameraProjectionMode::Perspective)
		{
			DrawFrustum->FrustumAngle = FieldOfView;
		}
		else
		{
			DrawFrustum->FrustumAngle = -FieldOfView;
		}
		DrawFrustum->FrustumStartDist = NearClipPlane;
		DrawFrustum->FrustumMidDist = HoloPlayPlane;
		DrawFrustum->FrustumEndDist = FarClipPlane;
		DrawFrustum->VerticalAngle = VertAngle;
		DrawFrustum->FrustumAspectRatio = AspectRatio;
		DrawFrustum->MidPlaneLineLength = MidPlaneLineLength;
		DrawFrustum->FrustumLineThickness = FrustumLineThickness;
		DrawFrustum->MidLineThickness = MidLineThickness;
		DrawFrustum->MarkRenderStateDirty();
	}
}

void UHoloPlayCameraComponent::OverrideFrustumColor(FColor OverrideColor, FColor OverrideMidColor)
{
	if (DrawFrustum != nullptr)
	{
		DrawFrustum->FrustumColor = OverrideColor;
		DrawFrustum->MidFrustumColor = OverrideMidColor;
	}
}

void UHoloPlayCameraComponent::RestoreFrustumColor()
{
	if (DrawFrustum != nullptr)
	{
		const FColor DefaultFrustumColor(255, 0, 255, 255);
		DrawFrustum->FrustumColor = DefaultFrustumColor;
		const FColor DefaultMidFrustumColor(255, 255, 255, 255);
		DrawFrustum->MidFrustumColor = DefaultMidFrustumColor;
	}
}

void UHoloPlayCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	RefreshVisualRepresentation();
}


void UHoloPlayCameraComponent::CheckForErrors()
{
	Super::CheckForErrors();

	if (AspectRatio <= 0.0f)
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_CameraAspectRatioIsZero", "Camera has AspectRatio=0 - please set this to something non-zero")))
			->AddToken(FMapErrorToken::Create(FMapErrors::CameraAspectRatioIsZero));
	}
}

bool UHoloPlayCameraComponent::GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut)
{
	return false;
}
#endif	// WITH_EDITOR

#undef LOCTEXT_NAMESPACE




