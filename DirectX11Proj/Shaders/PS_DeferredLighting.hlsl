#include "Lighting.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D occlusionTexture : register(t3);
Texture2D depthMapTexture : register(t4);

SamplerState SamplePointClamp : register(s0);
SamplerState SamplePointWrap : register(s1);

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};


// Shadow mapping with percentage close filtering 
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
			float2 texelSize = float2(1.0f / shadowMapWidth, 1.0f / shadowMapHeight);
			float2 offsetProjTexCoord = projectTexCoord + float2(x, y) * texelSize;

			// Clamp the coordinates
			if ((saturate(offsetProjTexCoord.x) == offsetProjTexCoord.x) && saturate(offsetProjTexCoord.y) == offsetProjTexCoord.y)
			{
				depthValue = depthMapTexture.Sample(SamplePointClamp, offsetProjTexCoord).r;

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

float4 PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	float4 albedo = albedoTexture.Load(int3(quadIn.position.xy, 0));
	float3 position = (float3)positionTexture.Load(int3(quadIn.position.xy, 0));
	float3 normal = (float3)normalTexture.Load(int3(quadIn.position.xy, 0));
	float occlusion = occlusionTexture.Load(int3(quadIn.position.xy, 0)).r;

	// Calculate the world space position by multiplying by the inverse view matrix
	// position is in view space
	// Multiplied by the inverse view space will take it to world space
	float4 worldSpacePosition = mul(float4(position.xyz, 1.0f), viewMatrixInversed);

	// Transform the world space position to light projection space
	float4 lightViewPosition = mul(mul(float4(worldSpacePosition.xyz, 1.0f), lightViewMatrix), lightProjectionMatrix);
	
	float shadowImpact = ShadowMappingPCF(lightViewPosition);
	return PerformLightingDeferred((float3)position, (float3)normal, albedo, 1.0f, occlusion) * shadowImpact;

}