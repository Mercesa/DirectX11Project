Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

#include "Lighting.hlsl"

SamplerState SampleType;


struct VertexInputType
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
	float3 norm : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
	
};

PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
	
	output.position = input.position;
	output.position.w = 1.0f;
	
	output.uv = input.uv;
	output.norm = input.normal;
	output.fragPos = input.position;
	output.tang = input.tangent;


    return output;
}
