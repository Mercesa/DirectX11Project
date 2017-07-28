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


float4 PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
{ 
	float2 noiseScale = float2(1024.0f / 4.0f, 768.0f / 4.0f);
	// Loads a texture with the x and y screen position
	float4 albedo = albedoTexture.Load(int3(quadIn.position.xy, 0));
	float3 position = (float3)positionTexture.Load(int3(quadIn.position.xy, 0));
	float3 normal = (float3)normalTexture.Load(int3(quadIn.position.xy, 0));
	float3 randomVec = (float3)noiseTexture.Sample(SamplePointWrap, quadIn.texcoord * noiseScale);


	float3 tangent = normalize(randomVec - normal*dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	
	for (int i = 0; i < 64; ++i)
	{
		float3 samp = mul(kernelSamples[i], TBN);
		samp = position + samp * 0.5f;

		float4 offset = float4(samp.rgb, 1.0f);
		offset = mul(offset, projectionMatrix);
		
		offset.x /= offset.w;		
		offset.y /= -offset.w;
		offset.z /= offset.w;

		// transform to 0 - 1 range
		offset.xyz = offset.xyz * 0.5f + 0.5f;

		float sampleDepth = positionTexture.Sample(SamplePointWrap, offset.xy).z;
		float rangeCheck = smoothstep(0.0f, 1.0f, 0.5f / abs(position.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + 0.01f ? 1.0 : 0.0) * rangeCheck;
	}

	float4 newPosition = mul(float4(position.xyz, 1.0f), viewMatrixInversed);

	//position = float3(newPosition.xyz);
	float4 lightViewPosition = mul(mul(float4(newPosition.xyz, 1.0f), lightViewMatrix), lightProjectionMatrix);

	float bias;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float shadowValue = 0.0f;

	bias = 0.001f;
	// Project the coordinates and put them from -1 to 1   to   0 to 1
	projectTexCoord.x = lightViewPosition.x / lightViewPosition.w * 0.5f + 0.5f;
	projectTexCoord.y = lightViewPosition.y / -lightViewPosition.w * 0.5f + 0.5f;

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

				lightDepthValue = lightViewPosition.z / lightViewPosition.w;

				lightDepthValue -= bias;

				// If light value is less far away than the depth value
				shadowValue += depthValue > lightDepthValue ? 1.0f : 0.1f;
				continue;
			}
			shadowValue += 1.0f;
		}
	}
	shadowValue /= 16.0f;

	occlusion =	((occlusion / 64.0f));
	return PerformLightingDeferred((float3)position, (float3)normal, albedo, 1.0f, occlusion) * shadowValue;

}