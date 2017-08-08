#include "Lighting.hlsl"

Texture2D ambientOcclusionTexture : register(t0);


SamplerState SamplePointClamp : register(s0);
SamplerState SamplePointWrap : register(s1);



struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};


float PSDeferredLighting(VSQuadOut quadIn) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	float texCol = ambientOcclusionTexture.Sample(SamplePointClamp, quadIn.texcoord);

// Our weights for guassian blur
float weight[5] = {
	0.06136,	0.24477	,0.38774	,0.24477	,0.06136 };

	// Find the size of texel size
	float2 texelSize = float2(1.0f / 1024.0f, 1.0f/ 768.0f);
	
	float result = weight[0] * texCol;

	
	// Get surrounding samples horizontally or vertically, and add them together 
	if (blurHorizontal)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += ambientOcclusionTexture.Sample(SamplePointClamp, quadIn.texcoord + float2(i*texelSize.x, 0)).r * weight[i];//(int3 + float2(i, 0), 0)).r * weight[i];
			result += ambientOcclusionTexture.Sample(SamplePointClamp, quadIn.texcoord - float2(i*texelSize.x, 0)).r * weight[i];
		}
	}

	else
	{
		for (int i = 1; i < 5; ++i)
		{
			result += ambientOcclusionTexture.Sample(SamplePointClamp, quadIn.texcoord + float2(0, i * texelSize.y)).r * weight[i];
			result += ambientOcclusionTexture.Sample(SamplePointClamp, quadIn.texcoord - float2(0, i * texelSize.y)).r * weight[i];
		}
	}

	return result / 2.0f;
}