#include "Lighting.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);

SamplerState SamplePointClamp : register(s0);



struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};


float4 PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	float4 albedo = albedoTexture.Load(int3(quadIn.position.xy, 0));
	float4 position = positionTexture.Load(int3(quadIn.position.xy, 0));
	float4 normal = normalTexture.Load(int3(quadIn.position.xy, 0));

	 return PerformLighting((float3)position, (float3)normal, albedo, 1.0f);
}