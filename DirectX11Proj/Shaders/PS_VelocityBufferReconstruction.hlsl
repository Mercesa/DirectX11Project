#include "Lighting.hlsl"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 positionNow : POSITIONNOW;
	float4 positionPrev : POSITIONPREV;
	
};

int2 VelocityBufferReconstructionPixelShader(PixelInputType input) : SV_TARGET
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

	// 1/2 * distance * exposure time * framerate
	float2 distance = 0.5f * (positionNow.xy - positionPrev.xy) * 16.66 * framerate;

	//  (qx * max(0.5px, min( ||qx||, k)))  / length(qx) + epsilon
	
	int2 distanceCompressed = (distance*max(0.5, min(length(distance), 20))) / length(distance);

	return distanceCompressed;
}
