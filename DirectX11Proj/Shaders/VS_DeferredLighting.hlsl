
struct VSQuadOut {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

VSQuadOut VSDeferredLighting(uint VertexID: SV_VertexID) {// ouputs a full screen quad with tex coords
	VSQuadOut Out;
	
	Out.texcoord = float2(VertexID & 1, VertexID >> 1); //you can use these for texture coordinates later
	Out.position= float4((Out.texcoord.x - 0.5f) * 2, -(Out.texcoord.y - 0.5f) * 2, 0, 1);
	return Out;
}