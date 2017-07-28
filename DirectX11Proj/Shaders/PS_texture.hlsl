Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

#include "Lighting.hlsl"

SamplerState SampleType;


struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
	float3 norm : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
};


float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float3 tNorm = input.norm;
	if(hasNormal)
	{
		tNorm = NormalSampleToWorldSpace(normalTexture.Sample(SampleType, input.uv), input.norm, input.tang);
	}

	float4 specColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if(hasSpecular)
	{
		specColor = specularTexture.Sample(SampleType, input.uv);
	}

	float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	
	if (hasDiffuse)
	{
		diffuseColor = diffuseTexture.Sample(SampleType, input.uv);
	}
	return PerformLighting(input.fragPos, tNorm, diffuseColor, specColor, 1.0f);
}
