#include "Render/HoloPlayLenticularShader.h"

#include "Misc/HoloPlayLog.h"

#include "RHIStaticStates.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FVertexHoloPlayConstantParameters, "VSConstant");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelHoloPlayConstantParameters, "PSConstant");

FVertexBufferRHIRef FHoloPlayLenticularShader::CreateTempHoloPlayVertexBuffer(float ULeft, float URight, float VTop, float VBottom)
{
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FHoloPlayTextureVertex) * 4, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FHoloPlayTextureVertex) * 4, RLM_WriteOnly);

	FHoloPlayTextureVertex* Vertices = (FHoloPlayTextureVertex*)VoidPtr;
	Vertices[0].Position.Set(-1.0f, 1.0f, 1.0f, 1.0f); // Top Left
	Vertices[1].Position.Set(1.0f, 1.0f, 1.0f, 1.0f); // Top Right
	Vertices[2].Position.Set(-1.0f, -1.0f, 1.0f, 1.0f); // Bottom Left
	Vertices[3].Position.Set(1.0f, -1.0f, 1.0f, 1.0f); // Bottom Right

	Vertices[0].UV.Set(ULeft, VTop);
	Vertices[1].UV.Set(URight, VTop);
	Vertices[2].UV.Set(ULeft, VBottom);
	Vertices[3].UV.Set(URight, VBottom);
	RHIUnlockVertexBuffer(VertexBufferRHI);

	return VertexBufferRHI;
}

void FHoloPlayCopyToQuiltPS::SetParameters(FRHICommandList & RHICmdList, FTextureRHIParamRef Texture)
{
	FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

	FSamplerStateRHIParamRef SamplerStateRHI = TStaticSamplerState<SF_Point>::GetRHI();
	SetTextureParameter(RHICmdList, PixelShaderRHI, InTexture, InTextureSampler, SamplerStateRHI, Texture);
}

void FHoloPlayCopyToQuiltPS::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

	if (InTexture.IsBound())
	{
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, InTexture.GetBaseIndex(), FShaderResourceViewRHIParamRef());
	}
}

bool FHoloPlayCopyToQuiltPS::Serialize(FArchive & Ar)
{
	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);

	// Serialize shader inputs.
	Ar << InTexture;
	Ar << InTextureSampler;

	return bShaderHasOutdatedParameters;
}

void FHoloPlayLenticularShaderPS::SetParameters(FRHICommandList & RHICmdList, FTextureRHIParamRef Texture, FSamplerStateRHIParamRef SamplerState)
{
	FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

	SetTextureParameter(RHICmdList, PixelShaderRHI, InTexture, InTextureSampler, SamplerState, Texture);
}

void FHoloPlayLenticularShaderPS::SetUniformBuffers(FRHICommandList& RHICmdList, FPixelHoloPlayConstantParameters& PixelHoloPlayConstantParameters)
{
	FPixelHoloPlayConstantUniformBufferRef ConstantParametersBuffer;

	ConstantParametersBuffer = FPixelHoloPlayConstantUniformBufferRef::CreateUniformBufferImmediate(PixelHoloPlayConstantParameters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPixelHoloPlayConstantParameters>(), ConstantParametersBuffer);
}

bool FHoloPlayLenticularShaderPS::Serialize(FArchive& Ar)
{
	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);

	// Serialize shader inputs. 
	Ar << InTexture;
	Ar << InTextureSampler;

	return bShaderHasOutdatedParameters;
}

void FHoloPlayLenticularShaderPS::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

	if (InTexture.IsBound())
	{
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, InTexture.GetBaseIndex(), FShaderResourceViewRHIParamRef());
	}
}

IMPLEMENT_SHADER_TYPE(, FHoloPlayCopyToQuiltVS, TEXT("/Plugin/HoloPlay/Private/HoloPlayLenticularShader.usf"), TEXT("CopyToQuiltVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FHoloPlayCopyToQuiltPS, TEXT("/Plugin/HoloPlay/Private/HoloPlayLenticularShader.usf"), TEXT("CopyToQuiltPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FHoloPlayLenticularShaderVS, TEXT("/Plugin/HoloPlay/Private/HoloPlayLenticularShader.usf"), TEXT("LenticularVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FHoloPlayLenticularShaderPS, TEXT("/Plugin/HoloPlay/Private/HoloPlayLenticularShader.usf"), TEXT("LenticularPS"), SF_Pixel);
