
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
	float4 positionNow : POSITIONNOW;
	float4 positionPrev : POSITIONPREV;
};

PixelInputType VelocityVertexShader(VertexInputType input)
{
	PixelInputType output;

	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, projViewMatrix);

	output.positionNow = output.position;

	output.positionPrev = mul(input.position, prevWorldMatrix);
	output.positionPrev = mul(output.positionPrev, prevProjViewMatrix);


	return output;
}
