Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

#include "Lighting.hlsl"

SamplerState SampleType;





struct VertexInputType
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 bitang : BITANGENT;
	float3 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
	float3 norm : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGOUT;
	
};

PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
	output.fragPos = output.position;
	
	output.norm = mul(float4(input.normal, 1.0), worldMatrix);
	output.tang = mul(float4(input.tangent, 1.0), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
        
	output.uv = input.uv;


    return output;
}

float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float3 tNorm = input.norm;
	if(hasNormal)
	{
		tNorm = NormalSampleToWorldSpace(normalTexture.Sample(SampleType, input.uv), input.norm, input.tang);
	}

	float4 specColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if(hasSpecular)
	{
		specColor = specularTexture.Sample(SampleType, input.uv);
	}

	return PerformLighting(input.fragPos, tNorm, diffuseTexture.Sample(SampleType, input.uv), specColor);
}
