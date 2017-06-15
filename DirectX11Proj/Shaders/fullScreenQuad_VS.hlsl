

struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

VSQuadOut VertShader(uint VertexID: SV_VertexID) {// ouputs a full screen quad with tex coords
	VSQuadOut Out;
	Out.texcoord = float2((VertexID << 1) & 2, VertexID & 2);
	Out.position = float4(Out.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Out;
}