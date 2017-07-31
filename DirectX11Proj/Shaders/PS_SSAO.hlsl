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


float PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
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

	occlusion =	((occlusion / 64.0f));
	return occlusion;

}