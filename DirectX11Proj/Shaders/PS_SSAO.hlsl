#include "Lighting.hlsl"

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
	float2 noiseScale = float2(screenWidth / 4.0f, screenHeight/ 4.0f);

	// Loads a texture with the x and y screen position
	// Position, normal and randomvec are all in viewspace
	float3 position = (float3)positionTexture.Load(int3(quadIn.position.xy, 0));
	float3 normal = (float3)normalTexture.Load(int3(quadIn.position.xy, 0));
	float3 randomVec = (float3)noiseTexture.Sample(SamplePointWrap, quadIn.texcoord * noiseScale);

	// Create random tangent vector by using gramm schmidt-orthonormalization 
	float3 tangent = normalize(randomVec - normal*dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	float radius = 0.5f;
	float bias = 0.01f;
	for (int i = 0; i < 32; ++i)
	{
		// Multiply kernel samples (tangent space) with TBN to bring it to view space
		float3 samp = mul(kernelSamples[i], TBN);
		samp = position + samp * radius;

		float4 offset = float4(samp.rgb, 1.0f);

		// transform to clip space
		offset = mul(offset, projectionMatrix);
		
		// Project coordinates
		offset.x /= offset.w;	
		offset.y /= -offset.w;

		// transform to 0 - 1 range
		offset.xy = offset.xy * 0.5f + 0.5f;

		
		float sampleDepth = positionTexture.Sample(SamplePointWrap, offset.xy).z;
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(position.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion =	((occlusion / 32.0f));
	return occlusion;

}