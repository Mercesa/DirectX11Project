#include "Lighting.hlsl"

Texture2D<int> velocityTexture : register(t0);

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};


int2 GetMaxVector(int2 a, int2 b)
{
	float lA = length(a);
	float lB = length(b);

	if (lA >= lB)
	{
		return a;
	}

	return b;
}

int2 VelocityBufferReconstructionPixelShader(VSQuadOut quadIn) : SV_TARGET
{
	//return velocityTexture.Load(int3(quadIn.position.xy, 0));

	float2 texCoords = quadIn.position.xy / float2(screenWidth, screenHeight);
	int2 currMax = int2(0, 0);
	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 20; ++j)
		{
			currMax = GetMaxVector(velocityTexture.Load(int3(texCoords.xy * int2(20,20) + int2(i,j), 0.)), currMax).rg;
		}
	}
	return int2(1, 1);
	return currMax;
}
