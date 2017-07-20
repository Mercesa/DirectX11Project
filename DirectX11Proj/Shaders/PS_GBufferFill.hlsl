#include "Lighting.hlsl"

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

Texture2D depthMapTexture : register(t3);

SamplerState SampleTypeClamp : register(s0);
SamplerState SampleTypeWrap : register(s1);
SamplerState SampleTypeAnisotropicWrap : register(s2);
SamplerState SampleTypeLinearWrap : register(s3);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 lightViewPosition : TEXCOORD1;
	float3 normal : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
	float2 tex : TEXCOORD0;

};

struct PS_OUT
{
	float4 albedo : SV_Target0;
	float4 normal : SV_Target1;
	float4 position : SV_Target2;
};


PS_OUT GbufferFillPixelShader(PixelInputType input) : SV_Target
{
	PS_OUT o;

	o.albedo = diffuseTexture.Sample(SampleTypeAnisotropicWrap, input.tex);
	o.normal = diffuseTexture.Sample(SampleTypeAnisotropicWrap, input.tex);


	if (hasDiffuse)
	{
		o.albedo = diffuseTexture.Sample(SampleTypeAnisotropicWrap, input.tex);
	}
	
	else
	{
		o.albedo = float4(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (hasNormal)
	{
		o.normal = float4(NormalSampleToWorldSpace(normalTexture.Sample(SampleTypeLinearWrap, input.tex), input.normal, input.tang).rgb, 0.0f);
	}

	else
	{
		o.normal = float4(input.normal.rgb, 0.0f);
	}

	o.position = float4(input.fragPos.rgb, 1.0f);

	return o;
}
