#include "Lighting.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D noiseTexture : register(t3);
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
			// Hardcode the resolution for now
			float texelSize = 1.0f / 8096.0f;
			float2 offsetProjTexCoord = projectTexCoord + float2(x*texelSize, y*texelSize);

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

// Screen space ambient occlusion function
float SSAO(float3 aViewSpacePosition, float3 aViewSpaceNormal, float2 aTexCoord)
{
	float2 noiseScale = float2(1024.0f / 4.0f, 768.0f / 4.0f);
	float3 randomVec = (float3)noiseTexture.Sample(SamplePointWrap, aTexCoord * noiseScale);

	float3 tangent = normalize(randomVec - aViewSpaceNormal*dot(randomVec, aViewSpaceNormal));
	float3 bitangent = cross(aViewSpaceNormal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, aViewSpaceNormal);

	float occlusion = 0.0f;

	for (int i = 0; i < 64; ++i)
	{
		float3 samp = mul(kernelSamples[i], TBN);
		samp = aViewSpacePosition + samp * 0.5f;

		float4 offset = float4(samp.rgb, 1.0f);
		offset = mul(offset, projectionMatrix);

		offset.x /= offset.w;
		offset.y /= -offset.w;
		offset.z /= offset.w;

		// transform to 0 - 1 range
		offset.xyz = offset.xyz * 0.5f + 0.5f;

		float sampleDepth = positionTexture.Sample(SamplePointWrap, offset.xy).z;
		float rangeCheck = smoothstep(0.0f, 1.0f, 0.5f / abs(aViewSpacePosition.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + 0.01f ? 1.0 : 0.0) * rangeCheck;
	}
	return occlusion;
}


float4 PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	float4 albedo = albedoTexture.Load(int3(quadIn.position.xy, 0));
	float3 position = (float3)positionTexture.Load(int3(quadIn.position.xy, 0));
	float3 normal = (float3)normalTexture.Load(int3(quadIn.position.xy, 0));

	float occlusion = SSAO(position, normal, quadIn.texcoord);

	
	// Calculate the world space position by multiplying by the inverse view matrix
	// position is in view space
	// Multiplied by the inverse view space will take it to world space
	float4 worldSpacePosition = mul(float4(position.xyz, 1.0f), viewMatrixInversed);

	// Transform the world space position to light projection space
	float4 lightViewPosition = mul(mul(float4(worldSpacePosition.xyz, 1.0f), lightViewMatrix), lightProjectionMatrix);
	
	float shadowImpact = ShadowMappingPCF(lightViewPosition);

	occlusion =	((occlusion / 64.0f));
	return PerformLightingDeferred((float3)position, (float3)normal, albedo, 1.0f, occlusion) * shadowImpact;

}