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


float4 ShadowPixelShader(PixelInputType input) : SV_TARGET
{
	float bias;
	float4 color;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float4 textureColor;
	float shadowValue = 0.0f;
	color = 0;

	bias = 0.0001f;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = input.lightViewPosition.y / -input.lightViewPosition.w / 2.0f + 0.5f;

	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && saturate(projectTexCoord.y) == projectTexCoord.y)
	{
		float texelSize = 1.0f / 1024.0f;

		depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		lightDepthValue -= bias;

		// If light value is less far away than the depth value
		shadowValue += depthValue > lightDepthValue ? 1.0f : 0.0f;
		
	}

	color = PerformLighting(input.fragPos, input.normal, float4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

	color *= shadowValue;
	return color;
}