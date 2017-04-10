Texture2D shaderTexture;
SamplerState SampleType;

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

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
};

PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
        
	output.uv = input.uv;

    return output;
}

float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
    return shaderTexture.Sample(SampleType, input.uv);
}
