#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "ShaderParameterUtils.h"
#include "Containers/DynamicRHIResourceArray.h"

/**
 * @struct	FHoloPlayTextureVertex
 *
 * @brief	A holo play texture vertex.
 * 			Containts Position and UV data
 */

struct FHoloPlayTextureVertex
{
	FVector4 Position;
	FVector2D UV;
};

/**
 * @class	FHoloPlayVertexBuffer
 *
 * @brief	We define our vertex declaration to let us get our UV coords into the shader
 */

class FHoloPlayVertexBuffer : public FVertexBuffer
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/**
	 * @fn	virtual void FHoloPlayVertexBuffer::InitRHI() override
	 *
	 * @brief	Initializes the rhi Vertex Buffers
	 */

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FHoloPlayTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHoloPlayTextureVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHoloPlayTextureVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	/**
	 * @fn	virtual void FHoloPlayVertexBuffer::ReleaseRHI() override
	 *
	 * @brief	Releases the Vertex Buffers
	 */

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/**
 * @class	FHoloPlayIndexBuffer
 *
 * @brief	HoloPlay index buffer
 */

class FHoloPlayIndexBuffer : public FIndexBuffer
{
public:

	/**
	 * @fn	void FHoloPlayIndexBuffer::InitRHI() override
	 *
	 * @brief	Initialize the RHI for this rendering resource
	 */

	void InitRHI() override
	{
		// Indices 0 - 5 are used for rendering a quad. Indices 6 - 8 are used for triangle optimization.
		const uint16 Indices[] = { 0, 1, 2, 2, 1, 3, 0, 4, 5 };

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		// Create index buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};


// HoloPlay vertex shader params
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVertexHoloPlayConstantParameters, )
END_GLOBAL_SHADER_PARAMETER_STRUCT()

// HoloPlay pixel shader params
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelHoloPlayConstantParameters,)
SHADER_PARAMETER(float, pitch)
SHADER_PARAMETER(float, slope)
SHADER_PARAMETER(float, center)
SHADER_PARAMETER(float, subp)
SHADER_PARAMETER(FVector4, tile)
SHADER_PARAMETER(FVector2D, viewPortion)
SHADER_PARAMETER(FVector4, aspect)
SHADER_PARAMETER(int, QuiltMode)
SHADER_PARAMETER(int, FlipYTexCoords)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FVertexHoloPlayConstantParameters> FVertexHoloPlayConstantUniformBufferRef;
typedef TUniformBufferRef<FPixelHoloPlayConstantParameters> FPixelHoloPlayConstantUniformBufferRef;

/**
 * @class	FHoloPlayLenticularShader
 *
 * @brief	A holo play lenticular shader.
 * 			Parent shader
 */

class FHoloPlayLenticularShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FHoloPlayLenticularShader, Global, /*HOLOPLAYRUNTIME_API*/);

	FHoloPlayLenticularShader() { }
	FHoloPlayLenticularShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

public:

	/**
	 * @fn	static bool FHoloPlayLenticularShader::ShouldCache(EShaderPlatform Platform)
	 *
	 * @brief	Determine if we should cache
	 *
	 * @param	Platform	The platform.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	/**
	 * @fn	static bool FHoloPlayLenticularShader::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	 *
	 * @brief	Useful when adding a permutation of a particular shader
	 *
	 * @param	Parameters	Options for controlling the operation.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	/**
	 * @fn	static FVertexBufferRHIRef FHoloPlayLenticularShader::CreateTempHoloPlayVertexBuffer(float ULeft = 0.0f, float URight = 1.0f, float VTop = 0.0f, float VBottom = 1.0f);
	 *
	 * @brief	Creates temporary holo play vertex buffer
	 * 			It create and bind vertex buffer on GPU
	 *
	 * @param	ULeft  	(Optional) The left.
	 * @param	URight 	(Optional) The right.
	 * @param	VTop   	(Optional) The top.
	 * @param	VBottom	(Optional) The bottom.
	 *
	 * @returns	The new holo play vertex buffer.
	 */

	static FVertexBufferRHIRef CreateTempHoloPlayVertexBuffer(float ULeft = 0.0f, float URight = 1.0f, float VTop = 0.0f, float VBottom = 1.0f);
};

/**
 * @class	FHoloPlayCopyToQuiltVS
 *
 * @brief	Helper Vertex shader for copy tiling texture to quilt. This class cannot be inherited..
 */

class FHoloPlayCopyToQuiltVS final : public FHoloPlayLenticularShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FHoloPlayCopyToQuiltVS, Global, /*HOLOPLAYRUNTIME_API*/);

public:
	FHoloPlayCopyToQuiltVS() { }
	FHoloPlayCopyToQuiltVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FHoloPlayLenticularShader(Initializer)
	{
	}
};

/**
 * @class	FHoloPlayCopyToQuiltPS
 *
 * @brief	Helper Pixel shader for copy tiling texture to quilt. This class cannot be inherited..
 */

class FHoloPlayCopyToQuiltPS final : public FHoloPlayLenticularShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FHoloPlayCopyToQuiltPS, Global, /*HOLOPLAYRUNTIME_API*/);

public:
	FHoloPlayCopyToQuiltPS() { }
	FHoloPlayCopyToQuiltPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FHoloPlayLenticularShader(Initializer)
	{
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
	}

	/**
	 * @fn	void final::SetParameters(FRHICommandList& RHICmdList, FTextureRHIParamRef Texture);
	 *
	 * @brief	Sets the constants parameters to the shader
	 *
	 * @param [in,out]	RHICmdList	List of rhi commands.
	 * @param 		  	Texture   	The texture resource on GPU
	 */

	void SetParameters(FRHICommandList& RHICmdList, FTextureRHIParamRef Texture);

	/**
	 * @fn	void final::UnbindBuffers(FRHICommandList& RHICmdList);
	 *
	 * @brief	Unbind GPU allocated buffers
	 *
	 * @param [in,out]	RHICmdList	List of rhi commands.
	 */

	void UnbindBuffers(FRHICommandList& RHICmdList);

private:

	/**
	 * @fn	virtual bool final::Serialize(FArchive& Ar) override;
	 *
	 * @brief	Serialize this object to the given stream
	 *
	 * @param [in,out]	Ar	The archive.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */

	virtual bool Serialize(FArchive& Ar) override;

private:
	FShaderResourceParameter InTexture;
	FShaderResourceParameter InTextureSampler;
};

/**
 * @class	FHoloPlayLenticularShaderVS
 *
 * @brief	CPP implementation HoloPlay Lenticular Vertex shader. This class cannot be inherited..
 */

class FHoloPlayLenticularShaderVS final : public FHoloPlayLenticularShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FHoloPlayLenticularShaderVS, Global, /*HOLOPLAYRUNTIME_API*/);

public:
	FHoloPlayLenticularShaderVS() { }
	FHoloPlayLenticularShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FHoloPlayLenticularShader(Initializer)
	{
	}
};

/**
 * @class	FHoloPlayLenticularShaderPS
 *
 * @brief	CPP implementation HoloPlay Lenticular Vertex shader. This class cannot be inherited..
 */

class FHoloPlayLenticularShaderPS final : public FHoloPlayLenticularShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FHoloPlayLenticularShaderPS, Global, /*HOLOPLAYRUNTIME_API*/);

public:
	FHoloPlayLenticularShaderPS() { }
	FHoloPlayLenticularShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FHoloPlayLenticularShader(Initializer)
	{
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
	}

	/**
	 * @fn	void final::SetParameters(FRHICommandList& RHICmdList, FTextureRHIParamRef Texture, FSamplerStateRHIParamRef SamplerState);
	 *
	 * @brief	Sets the constants parameters to the shader
	 *
	 * @param [in,out]	RHICmdList  	List of rhi commands.
	 * @param 		  	Texture			The texture.
	 * @param 		  	SamplerState	State of the sampler.
	 */

	void SetParameters(FRHICommandList& RHICmdList, FTextureRHIParamRef Texture, FSamplerStateRHIParamRef SamplerState);

	/**
	 * @fn	void final::SetUniformBuffers(FRHICommandList& RHICmdList, FPixelHoloPlayConstantParameters& PixelHoloPlayConstantParameters);
	 *
	 * @brief	Sets uniform buffers to the shader
	 *
	 * @param [in,out]	RHICmdList					   	List of rhi commands.
	 * @param 		  	PixelHoloPlayConstantParameters	Options for controlling the pixel holo play
	 * 													constant.
	 */

	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelHoloPlayConstantParameters& PixelHoloPlayConstantParameters);

	/**
	 * @fn	void final::UnbindBuffers(FRHICommandList& RHICmdList);
	 *
	 * @brief	Unbind allocated buffers
	 *
	 * @param [in,out]	RHICmdList	List of rhi commands.
	 */

	void UnbindBuffers(FRHICommandList& RHICmdList);

private:

	/**
	 * @fn	virtual bool final::Serialize(FArchive& Ar) override;
	 *
	 * @brief	Serialize this object to the given stream
	 *
	 * @param [in,out]	Ar	The archive.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */

	virtual bool Serialize(FArchive& Ar) override;

private:
	FShaderResourceParameter InTexture;
	FShaderResourceParameter InTextureSampler;
};