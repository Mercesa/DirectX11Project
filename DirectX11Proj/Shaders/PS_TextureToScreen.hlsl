#include "Lighting.hlsl"

Texture2D textureToRender : register(t0);

SamplerState SampleLinearWrap : register(s0);

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

float4 TextureToScreenPixelShader(VSQuadOut quadIn) : SV_TARGET
{
	float2 texCoord = (quadIn.position + float2(0.0f, (0.0f))) / float2(320, 320);
	return textureToRender.Sample(SampleLinearWrap, texCoord);
}