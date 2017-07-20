#include "Lighting.hlsl"

Texture2D shaderTexture : register(t0);

Texture2D depthMapTexture : register(t1);

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


float4 ShadowPixelShader(PixelInputType input) : SV_TARGET
{
	float bias;
	float4 color;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float4 textureColor;

	color = 0;

	bias = 0.001f;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = input.lightViewPosition.y / -input.lightViewPosition.w / 2.0f + 0.5f;

	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && saturate(projectTexCoord.y) == projectTexCoord.y)
	{
		depthValue = depthMapTexture.Sample(SampleTypeWrap, projectTexCoord);

		lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		lightDepthValue - bias;


		// If light value is less far away than the depth value
		if (depthValue > lightDepthValue)
		{
			color = PerformLighting(input.fragPos, input.normal, shaderTexture.Sample(SampleTypeClamp, input.tex), 1.0f);
		}

	}

	
	return color;
}