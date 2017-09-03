struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
	float3 norm : NORMAL;
	float3 fragPos : FRAGPOSITION;
	float3 tang : TANGENT;
};


float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
}
