

Texture2D diffuseTexture : register(t0);

SamplerState SamplerPointWrap;

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};
float4 PixShader(VSQuadOut quadIn) : SV_TARGET
{ 
	//return float4(1.0f, 0.0f, 1.0f, 1.0f);
	return diffuseTexture.Sample(SamplerPointWrap, quadIn.texcoord); //the red color
}