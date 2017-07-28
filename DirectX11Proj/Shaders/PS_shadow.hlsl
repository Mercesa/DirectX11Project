#include "Lighting.hlsl"

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

Texture2D depthMapTexture : register(t3);

SamplerState SampleTypeClamp : register(s0);
SamplerState SampleTypeWrap : register(s1);
SamplerState SampleTypeAnisotropicWrap : register(s2);
SamplerState SampleTypeLinearWrap : register(s3);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 lightViewPosition : TEXCOORD1;
	float3 normal : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
	float2 tex : TEXCOORD0;
};


float ShadowMappingPCF(float4 aLightviewPosition)
{
	float bias;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float shadowValue = 0.0f;

	bias = 0.001f;
	// Project the coordinates and put them from -1 to 1   to   0 to 1
	projectTexCoord.x = aLightviewPosition.x / aLightviewPosition.w * 0.5f + 0.5f;
	projectTexCoord.y = aLightviewPosition.y / -aLightviewPosition.w * 0.5f + 0.5f;

	// Percentage close filtering
	for (int y = -2; y < 2; ++y)
	{
		for (int x = -2; x < 2; ++x)
		{
			// Hardcode the resolution for now
			float texelSize = 1.0f / 8096.0f;
			float2 offsetProjTexCoord = projectTexCoord + float2(x*texelSize, y*texelSize);
			
			// Clamp the coordinates
			if ((saturate(offsetProjTexCoord.x) == offsetProjTexCoord.x) && saturate(offsetProjTexCoord.y) == offsetProjTexCoord.y)
			{
				depthValue = depthMapTexture.Sample(SampleTypeClamp, offsetProjTexCoord).r;

				lightDepthValue = aLightviewPosition.z / aLightviewPosition.w;

				lightDepthValue -= bias;

				// If light value is less far away than the depth value
				shadowValue += depthValue > lightDepthValue ? 1.0f : 0.1f;
				continue;
			}
			shadowValue += 1.0f;
		}
	}
	shadowValue /= 16.0f;

	return shadowValue;
}

float4 ShadowPixelShader(PixelInputType input) : SV_TARGET
{
	float shadowImpact = ShadowMappingPCF(input.lightViewPosition);

	float3 tNorm = input.normal;
	if (hasNormal)
	{
		tNorm = NormalSampleToWorldSpace(normalTexture.Sample(SampleTypeLinearWrap, input.tex), input.normal , input.tang);
	}

	float4 specColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (hasSpecular)
	{
		specColor = specularTexture.Sample(SampleTypeAnisotropicWrap, input.tex);
	}

	float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (hasDiffuse)
	{
		diffuseColor = diffuseTexture.Sample(SampleTypeAnisotropicWrap, input.tex);
	}
	
	float4 color = PerformLighting(input.fragPos, tNorm, diffuseColor, 1.0f, 1.0f) * shadowImpact;

	return color;
}