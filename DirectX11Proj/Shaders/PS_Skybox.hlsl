

TextureCube skyboxTexture : register(t0);

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MAP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 positionUV : TEXCOORD0;
};

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
	return skyboxTexture.Sample(samTriLinearSam, input.positionUV);
}
