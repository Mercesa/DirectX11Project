#include "Lighting.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D occlusionTexture : register(t3);
Texture2D depthMapTexture : register(t4);
Texture2D velocityTexture : register(t5);

SamplerState SamplePointClamp : register(s0);
SamplerState SamplePointWrap : register(s1);

//struct VSQuadOut {
//	float4 position : SV_Position;
//	float2 texcoord: TexCoord;
//};
struct PixelInputType
{
	float4 position : SV_POSITION;

};

// Shadow mapping with percentage close filtering 
inline float ShadowMappingPCF(float4 aLightviewPosition)
{
	float bias;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float shadowValue = 0.0f;

	bias = 0.001f;
	// transform from -w to w by perspective division
	// Range transform -1,1 to 0,1
	projectTexCoord.x = aLightviewPosition.x / aLightviewPosition.w * 0.5f + 0.5f;

	// Y is inversed, as such we divide by -pos.w
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

				// Perspective divide
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

float4 PSDeferredLighting(PixelInputType input) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	//float4 albedo = albedoTexture.Load(int3(quadIn.position.xy, 0));
	//float3 position = (float3)positionTexture.Load(int3(quadIn.position.xy, 0));
	//float3 normal = (float3)normalTexture.Load(int3(quadIn.position.xy, 0));
	//float occlusion = occlusionTexture.Load(int3(quadIn.position.xy, 0)).r;

	float2 texCoord = input.position / float2(screenWidth, screenHeight);

	float4 albedo = albedoTexture.Sample(SamplePointClamp, texCoord);
	float3 position = positionTexture.Sample(SamplePointClamp, texCoord);
	float3 normal = normalTexture.Sample(SamplePointClamp, texCoord);
	float occlusion = occlusionTexture.Sample(SamplePointClamp, texCoord);

	// Calculate the world space position by multiplying by the inverse view matrix
	// position is in view space
	// Multiplied by the inverse view space will take it to world space
	float4 worldSpacePosition = mul(float4(position.xyz, 1.0f), viewMatrixInversed);



	// Next code is for motion blur
	// Calculate the difference between screen space positions
	
	// Transform world poisition with previous camera projection/view matrix
	// Project to screen, and calculate difference between 
	//float4 previous = mul(worldSpacePosition, prevProjViewMatrix);
	//previous.x /= previous.w;
	//previous.y /= -previous.w;
	//
	//previous.xy = previous.xy * 0.5f + 0.5f;
	//
	//float2 blurVec = (previous.xy - texCoord) *2.0f;

	float2 blurVec = float2(velocityTexture.Sample(SamplePointClamp, texCoord).rg) * 2.0f;
	float4 result = albedoTexture.Sample(SamplePointClamp, texCoord);

	int amountOfSamples = 6;
	for (int i = 1; i < amountOfSamples; ++i)
	{
		float2 offset = blurVec * (float(i) / float(amountOfSamples - 1) - 0.5);
		result += albedoTexture.Sample(SamplePointClamp, texCoord + offset);
	}

	result /= float(amountOfSamples);
	// Transform the world space position to light projection space
	float4 lightViewPosition = mul(mul(float4(worldSpacePosition.xyz, 1.0f), lightViewMatrix), lightProjectionMatrix);


	
	float shadowImpact = ShadowMappingPCF(lightViewPosition);
	//return float4(blurVec.xy * 1., 0.0f, 1.0f);
	return PerformLightingDeferred((float3)position, (float3)normal, albedo, 1.0f, occlusion) * shadowImpact;

}