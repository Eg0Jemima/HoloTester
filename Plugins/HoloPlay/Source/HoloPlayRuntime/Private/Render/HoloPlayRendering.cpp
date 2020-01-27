
#include "Render/HoloPlayRendering.h"
#include "Render/HoloPlayLenticularShader.h"

#include "Misc/HoloPlayLog.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "TextureResource.h"
#include "Engine.h"

/** Vertex declaration for the 2D screen rectangle. */
static TGlobalResource<FHoloPlayVertexBuffer> GHoloPlayVertexDeclaration;
static TGlobalResource<FHoloPlayIndexBuffer> GHoloPlayIndexDeclaration;

void HoloPlay::CopyToQuiltShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FRenderContext& Context, int CurrentView)
{
	check(IsInRenderingThread());

	SCOPED_DRAW_EVENTF(RHICmdList, Scene, TEXT("CopyToQuiltShader_RenderThread CurrentView %d"), CurrentView);
	DISPLAY_HOLOPLAY_FUNC_TRACE(HoloPlayLogRender)

	auto TilingValues = Context.TilingValues;

	// Set Render targets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FRHIRenderPassInfo RPInfo(Context.OverrideQuiltResource->GetRenderTargetTexture(), ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("CopyToQuiltShader_RenderThread"));

	// Set Viewport for tiling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	{
		int RI = TilingValues.NumViews - CurrentView - 1;
		int X = (CurrentView % TilingValues.TilesX) * TilingValues.TileSizeX;
		int Y = (RI / TilingValues.TilesX) * TilingValues.TileSizeY;

		// the padding is necessary because the shader takes y from the opposite spot as this does
		FVector2D Min(X, Y + TilingValues.PaddingY);
		FVector2D Max(TilingValues.TileSizeX + Min.X, TilingValues.TileSizeY + Min.Y);

		UE_LOG(HoloPlayLogRender, Verbose, TEXT("CurrentView %d, Min %s Max %s"), CurrentView, *Min.ToString(), *Max.ToString());

		RHICmdList.SetViewport(Min.X, Min.Y, 0.f, Max.X, Max.Y, 1.f);
	}

	// Get shaders. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const auto FeatureLevel = GMaxRHIFeatureLevel;
	TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef< FHoloPlayCopyToQuiltVS > VertexShader(GlobalShaderMap);
	TShaderMapRef< FHoloPlayCopyToQuiltPS > PixelShader(GlobalShaderMap);

	// Set the graphic pipeline state. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GHoloPlayVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	// Set the graphic pipeline state. END --------------------------------------

	// Update shader uniform parameters. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Update shader uniform parameters. END --------------------------------------

	// Update shader parameters and resources parameters. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	PixelShader->SetParameters(RHICmdList, Context.TilingTextureResource->TextureRHI);
	// Update shader parameters and resources parameters. END --------------------------------------

	// Set geometry START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FVertexBufferRHIRef VertexBufferRHI = FHoloPlayLenticularShader::CreateTempHoloPlayVertexBuffer();
	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	// Set geometry END --------------------------------------

	// Issue Indexed draw call ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	RHICmdList.DrawIndexedPrimitive(
		GHoloPlayIndexDeclaration.IndexBufferRHI,
		PT_TriangleList,
		/*BaseVertexIndex=*/ 0,
		/*MinIndex=*/ 0,
		/*NumVertices=*/ 4,
		/*StartIndex=*/ 0,
		/*NumPrimitives=*/ 2,
		/*NumInstances=*/ EDRF_Default
	);

	RHICmdList.CopyToResolveTarget(Context.OverrideQuiltResource->GetRenderTargetTexture(), Context.OverrideQuiltResource->TextureRHI, FResolveParams());

	PixelShader->UnbindBuffers(RHICmdList);

	RHICmdList.EndRenderPass();
}

void HoloPlay::RenderLenticularShader_RenderThread(FRHICommandListImmediate & RHICmdList, const FRenderContext& Context)
{
	check(IsInRenderingThread());

	SCOPED_DRAW_EVENTF(RHICmdList, Scene, TEXT("RenderLenticularShader_RenderThread"));
	DISPLAY_HOLOPLAY_FUNC_TRACE(HoloPlayLogRender)

	auto TilingValues = Context.TilingValues;
	float Aspect = (float)Context.Viewport->GetSizeXY().X / (float)Context.Viewport->GetSizeXY().Y;

	FTexture2DRHIRef RenderTargetTexture = Context.Viewport->GetRenderTargetTexture();
	auto HoloPlaySettings = Context.HoloPlaySettingsDuplicate;

	if (!RenderTargetTexture.IsValid())
	{
		return;
	}

	// Set render target ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FRHIRenderPassInfo RPInfo(Context.Viewport->GetRenderTargetTexture(), ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("RenderLenticularShader_RenderThread"));

	// Set viewport ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	RHICmdList.SetViewport(0, 0, 0.f, RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY(), 1.f);

	// Get shaders. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const auto FeatureLevel = GMaxRHIFeatureLevel;
	TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef< FHoloPlayLenticularShaderVS > VertexShader(GlobalShaderMap);
	TShaderMapRef< FHoloPlayLenticularShaderPS > PixelShader(GlobalShaderMap);

	// Set the graphic pipeline state. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GHoloPlayVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	// Set the graphic pipeline state. END --------------------------------------

	// Update shader uniform parameters. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	{
		FPixelHoloPlayConstantParameters PixelHoloPlayCP;

		PixelHoloPlayCP.pitch = HoloPlaySettings->HoloPlayShaderSettings.Pitch.DefaultValue;
		PixelHoloPlayCP.slope = HoloPlaySettings->HoloPlayShaderSettings.slope.DefaultValue;
		PixelHoloPlayCP.center = HoloPlaySettings->HoloPlayShaderSettings.center.DefaultValue;

		float Subp = 1.f / (HoloPlaySettings->HoloPlayShaderSettings.ScreenWidth.DefaultValue * 3.f);
		Subp *= (HoloPlaySettings->HoloPlayShaderSettings.flipImageX.DefaultValue > 0) ? -1 : 1;
		PixelHoloPlayCP.subp = Subp;

		PixelHoloPlayCP.tile = FVector4
		(
			TilingValues.TilesX,
			TilingValues.TilesY,
			TilingValues.NumViews,
			TilingValues.TilesX * TilingValues.TilesY
		);
		PixelHoloPlayCP.viewPortion = FVector2D
		(
			TilingValues.PortionX,
			TilingValues.PortionY
		);
		PixelHoloPlayCP.aspect = FVector4
		(
			Aspect,
			Aspect,
			TilingValues.Overscan ? 1 : 0,
			0
		);

		UE_LOG(HoloPlayLogRender, Verbose, TEXT("pitch %f, slope %f, center %f, subp %f, tile %s, viewPortion %s, aspect %s"),
			PixelHoloPlayCP.pitch, PixelHoloPlayCP.slope, PixelHoloPlayCP.center,  PixelHoloPlayCP.subp,
			*PixelHoloPlayCP.tile.ToString(), *PixelHoloPlayCP.viewPortion.ToString(), *PixelHoloPlayCP.aspect.ToString());

		PixelHoloPlayCP.QuiltMode = HoloPlaySettings->HoloPlayShaderSettings.QuiltMode;
		PixelHoloPlayCP.FlipYTexCoords = HoloPlaySettings->HoloPlayShaderSettings.FlipYTexCoords;
		PixelShader->SetUniformBuffers(RHICmdList, PixelHoloPlayCP);
	}
	// Update shader uniform parameters. END --------------------------------------

	// Update shader parameters and resources parameters. START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (Context.OverrideOverrideQuiltResource != nullptr)
	{
		auto QuiltRTRHIRef = Context.OverrideOverrideQuiltResource->TextureRHI;
		auto SamplerStateRHI = Context.OverrideOverrideQuiltResource->SamplerStateRHI;

		PixelShader->SetParameters(RHICmdList, QuiltRTRHIRef, SamplerStateRHI);
	}
	else
	{
		if (Context.OverrideQuiltResource->TextureRHI.IsValid())
		{
			FSamplerStateRHIParamRef SamplerStateRHI = TStaticSamplerState<SF_Point>::GetRHI();
			PixelShader->SetParameters(RHICmdList, Context.OverrideQuiltResource->TextureRHI, SamplerStateRHI);
		}
	}
	// Update shader parameters and resources parameters. END --------------------------------------

	// Set geometry START ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FVertexBufferRHIRef VertexBufferRHI = FHoloPlayLenticularShader::CreateTempHoloPlayVertexBuffer(0.f, 1.f, 1.f, 0.f);
	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	// Set geometry END --------------------------------------

	// Issue Indexed draw call ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	RHICmdList.DrawIndexedPrimitive(
		GHoloPlayIndexDeclaration.IndexBufferRHI,
		PT_TriangleList,
		/*BaseVertexIndex=*/ 0,
		/*MinIndex=*/ 0,
		/*NumVertices=*/ 4,
		/*StartIndex=*/ 0,
		/*NumPrimitives=*/ 2,
		/*NumInstances=*/ EDRF_Default
	);

	RHICmdList.CopyToResolveTarget(Context.OverrideQuiltResource->GetRenderTargetTexture(), Context.OverrideQuiltResource->TextureRHI, FResolveParams());

	PixelShader->UnbindBuffers(RHICmdList);

	RHICmdList.EndRenderPass();
}