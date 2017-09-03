
#include "Lighting.hlsl"

struct VertexInputType
{
	float4 position : Position;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
	float2 tex : TEXCOORD0;
};

PixelInputType GBufferFillVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);

	// Fragpos is in view space
	output.fragPos = output.position;

	output.position = mul(output.position, projectionMatrix);

	// normal is calculated in view space
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	
	output.tang = mul(input.tangent, (float3x3)worldMatrix);
	

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}
