#pragma once

#include "HoloPlaySettings.h"

#include "CoreMinimal.h"
#include "RHI.h"

class FViewport;
class FTextureRenderTargetResource;
class FTextureResource;

namespace HoloPlay
{
	/**
	 * @struct	FRenderContext
	 *
	 * @brief	A render context.
	 * 			Structure for copy from Game Thread to Render Thread for rendering
	 */

	struct FRenderContext
	{
		const FViewport* Viewport;
		const FTextureRenderTargetResource* OverrideQuiltResource;
		FHoloPlayTilingQuality TilingValues;
		const FTextureResource* TilingTextureResource;
		const FTextureResource* OverrideOverrideQuiltResource;
		const UHoloPlaySettings* HoloPlaySettingsDuplicate;
	};

	/**
	 * @fn	void CopyToQuiltShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FRenderContext& Context, int CurrentView);
	 *
	 * @brief	Copies to quilt
	 * 			It is copy one by one texture to the quilt render target from tiling texture
	 *
	 * @param [in,out]	RHICmdList 	List of rhi commands.
	 * @param 		  	Context	   	The context.
	 * @param 		  	CurrentView	The current view.
	 */

	void CopyToQuiltShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FRenderContext& Context, int CurrentView);

	/**
	 * @fn	void RenderLenticularShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FRenderContext& Context);
	 *
	 * @brief	Execute Lenticular with screen calibration data
	 *
	 * @param [in,out]	RHICmdList	List of rhi commands.
	 * @param 		  	Context   	The context.
	 */

	void RenderLenticularShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FRenderContext& Context);
}
