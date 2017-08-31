#include "Lighting.hlsl"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 positionNow : POSITIONNOW;
	float4 positionPrev : POSITIONPREV;
	
};

float2 VelocityPixelShader(PixelInputType input) : SV_TARGET
{
	float4 positionNow = input.positionNow;
	float4 positionPrev = input.positionPrev;

	// Perspective divide
	positionNow.xy /= float2(positionNow.w, -positionNow.w);

	// Transform towards -1 to 1 range from -0.5 to 0.5 range
	positionNow.xy *= float2(0.5f, 0.5f);
	positionNow.xy += float2(0.5f, 0.5f);

	// Perspective divide
	positionPrev.xy /= float2(positionPrev.w, -positionPrev.w);

	// Transform to -1 to 1 range from -0.5 to 0.5 range
	positionPrev.xy *= float2(0.5f, 0.5f);
	positionPrev.xy += float2(0.5f, 0.5f);

	return positionNow.xy - positionPrev.xy;
}
