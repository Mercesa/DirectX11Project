struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};


float4 DepthPixelShader(PixelInputType input) : SV_TARGET
{
	float depthValue;
	float4 color;
	depthValue = input.depthPosition.z / input.depthPosition.w;

	color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	return color;
}