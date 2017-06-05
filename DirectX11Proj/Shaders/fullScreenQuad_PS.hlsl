Texture2D renderedSceneTexture : register(t0);

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};
float4 PixShader(VSQuadOut quadIn) : SV_TARGET
{ 
	// Loads a texture with the x and y screen position
	return renderedSceneTexture.Load(int3(quadIn.position.xy, 0));
}