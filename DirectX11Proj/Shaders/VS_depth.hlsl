
#include "Lighting.hlsl"

struct VertexInputType
{
	float4 position : Position;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
};

PixelInputType DepthVertexShader(VertexInputType input)
{
	PixelInputType output;

	// transform with light matrices, can precalculate view and projection matrix as one
	output.position = mul(input.position, worldMatrix2);
	output.position = mul(output.position, lightViewMatrix);
	output.position = mul(output.position, lightProjectionMatrix);


	return output;
}
