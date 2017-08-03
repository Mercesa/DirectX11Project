
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
	float3 positionUV : TEXCOORD0;
};

PixelInputType SkyboxVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix).xyww;
        
	output.positionUV = input.position;

    return output;
}

