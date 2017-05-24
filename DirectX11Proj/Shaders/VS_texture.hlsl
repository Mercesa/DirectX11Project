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
	float3 bitang : BITANGENT;
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
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
	output.fragPos = output.position;
	
	output.norm = mul(input.normal, (float3x3)worldMatrix);
	output.tang = mul(input.tangent, (float3x3)worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
        
	output.uv = input.uv;


    return output;
}
