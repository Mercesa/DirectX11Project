#include "Lighting.hlsl"

Texture2D renderedSceneTexture : register(t0);

SamplerState SamplePointClamp : register(s0);


struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

float3 BoxBlur(float2 texCoord)
{

	float2 texelSize = float2(1.0f / screenWidth, 1.0f / screenHeight);

	float boxSize = 9.0f;

	float3 boxBlurResult = float3(0.0f, 0.0f, 0.0f);
	for (int y = -1; y < 2; ++y)
	{
		for (int x = -1; x < 2; ++x)
		{
			float2 offsetTexCoord = texCoord + texelSize * float2(x, y);
			boxBlurResult += renderedSceneTexture.Sample(SamplePointClamp, offsetTexCoord) * 0.111f;
		}
	}

	return boxBlurResult;
}

float Sobel(float2 texCoord)
{
	// Our two kernels
	float sobelHorizontal[] = { -1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f, 2.0f,
		-1.0f, 0.0f, 1.0f };

	float sobelVertical[] = { -1.0f, -2.0f, -1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 2.0f, 1.0f };

	// Calculate the texel size
	float2 texelSize = float2(1.0f / screenWidth, 1.0f / screenHeight);

	float3 xCol = 0.0f;
	float3 yCol = 0.0f;
	for (int y = 0; y < 3; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			// Calculate our offset texture coordinate
			float2 offsetTexCoord = texCoord + texelSize * float2(x-1, y-1);

			// Create our sobel value
			xCol += renderedSceneTexture.Sample(SamplePointClamp, offsetTexCoord) * sobelHorizontal[x + y * 3];
			yCol += renderedSceneTexture.Sample(SamplePointClamp, offsetTexCoord) * sobelVertical[x + y * 3];
		}
	}

	float sobelValue = sqrt(xCol*xCol + yCol*yCol);
	return sobelValue;
}



float4 PixShader(VSQuadOut quadIn) : SV_TARGET
{
	float sobelValue = Sobel(quadIn.texcoord);
	float3 boxBlurValue = BoxBlur(quadIn.texcoord);
	// Loads a texture with the x and y screen position
	return float4(sobelValue, sobelValue, sobelValue, 1.0f);//renderedSceneTexture.Load(int3(quadIn.position.xy + int2(2.0, 0.0), 0));

}