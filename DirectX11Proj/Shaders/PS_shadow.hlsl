#include "Lighting.hlsl"

Texture2D shaderTexture : register(t1);

Texture2D depthMapTexture : register(t3);

SamplerState SampleTypeClamp : register(s0);
SamplerState SampleTypeWrap : register(s1);


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1;
	float3 fragPos : FRAGPOSITION;
};


float ShadowMappingPCF(PixelInputType input)
{
	float bias;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float shadowValue = 0.0f;

	bias = 0.001f;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = input.lightViewPosition.y / -input.lightViewPosition.w / 2.0f + 0.5f;


	for (int y = -2; y < 2; ++y)
	{
		for (int x = -2; x < 2; ++x)
		{
			// Hardcode the resolution for now

			float texelSize = 1.0f / 2048.0f;
			float2 offsetProjTexCoord = float2(x*texelSize, y * texelSize);
			if ((saturate(offsetProjTexCoord.x) == offsetProjTexCoord.x) && saturate(offsetProjTexCoord.y) == offsetProjTexCoord.y)
			{
				depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

				lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

				lightDepthValue -= bias;

				// If light value is less far away than the depth value
				shadowValue += depthValue > lightDepthValue ? 1.0f : 0.0f;
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
	float shadowImpact = ShadowMappingPCF(input);
	float4 color = PerformLighting(input.fragPos, input.normal, float4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f) * shadowImpact;

	return color;
}