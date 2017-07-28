#include "Lighting.hlsl"

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);


SamplerState SampleTypeClamp : register(s0);
SamplerState SampleTypeWrap : register(s1);
SamplerState SampleTypeAnisotropicWrap : register(s2);
SamplerState SampleTypeLinearWrap : register(s3);

struct PixelInputType
{
	float4 position : SV_POSITION;
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

	if (hasDiffuse)
	{
		o.albedo = diffuseTexture.Sample(SampleTypeLinearWrap, input.tex);
	}
	
	else
	{
		o.albedo = float4(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (hasNormal)
	{
		// we put the w value of NormalSampleToWorldSpace to 0 since we are transforming a directio.
		float4 normalInViewSpace = mul(float4(NormalSampleToWorldSpace(normalTexture.Sample(SampleTypeLinearWrap, input.tex).rgb, input.normal, input.tang), 0.0f), viewMatrix);
		//normalInViewSpace = mul(normalInViewSpace.rgb, viewMatrix);
		o.normal = float4(normalize(normalInViewSpace.rgb), 0.0f);
	}

	else
	{
		o.normal = mul(float4(input.normal.rgb, 0.0f), viewMatrix);
	}

	o.position = float4(input.fragPos.rgb, 1.0f);
	return o;
}
