struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    return float4(input.uv.x, input.uv.y, 0.0, 1.0);
}
