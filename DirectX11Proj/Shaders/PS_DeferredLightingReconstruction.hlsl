#include "Lighting.hlsl"

Texture2D albedoTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D occlusionTexture : register(t3);
Texture2D depthMapTexture : register(t4);
Texture2D<int2> velocityTexture : register(t5);
Texture2D<int2> velocityNeighbourMaxTexture : register(t6);

SamplerState SamplePointClamp : register(s0);
SamplerState SamplePointWrap : register(s1);

//struct VSQuadOut {
//	float4 position : SV_Position;
//	float2 texcoord: TexCoord;
//};
struct PixelInputType
{
	float4 position : SV_POSITION;

};

// Shadow mapping with percentage close filtering 
float ShadowMappingPCF(float4 aLightviewPosition)
{
	float bias;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float shadowValue = 0.0f;

	bias = 0.001f;
	// transform from -w to w by perspective division
	// Range transform -1,1 to 0,1
	projectTexCoord.x = aLightviewPosition.x / aLightviewPosition.w * 0.5f + 0.5f;

	// Y is inversed, as such we divide by -pos.w
	projectTexCoord.y = aLightviewPosition.y / -aLightviewPosition.w * 0.5f + 0.5f;

	// Percentage close filtering
	for (int y = -2; y < 2; ++y)
	{
		for (int x = -2; x < 2; ++x)
		{
			float2 texelSize = float2(1.0f / shadowMapWidth, 1.0f / shadowMapHeight);
			float2 offsetProjTexCoord = projectTexCoord + float2(x, y) * texelSize;

			// Clamp the coordinates
			if ((saturate(offsetProjTexCoord.x) == offsetProjTexCoord.x) && saturate(offsetProjTexCoord.y) == offsetProjTexCoord.y)
			{
				depthValue = depthMapTexture.Sample(SamplePointClamp, offsetProjTexCoord).r;

				// Perspective divide
				lightDepthValue = aLightviewPosition.z / aLightviewPosition.w;

				lightDepthValue -= bias;

				// If light value is less far away than the depth value
				shadowValue += depthValue > lightDepthValue ? 1.0f : 0.1f;
				continue;
			}
			shadowValue += 1.0f;
		}
	}
	shadowValue /= 16.0f;

	return shadowValue;
}

float softDepthCompare(float firstZ, float secZ)
{
	return clamp(1.0f - (firstZ - secZ) / 1.0f, 0.0f, 1.0f);
}

float cone(float2 X, float2 Y, float2 v)
{
	return clamp(1 - length(X - Y) / length(v), 0, 1);
}

float cylinder(float2 X, float2 Y, float2 V)
{
	return 1.0 - smoothstep(0.95f*length(V), 1.05*length(V), length(X - Y));
}

float4 Reconstruct(float2 texCoord, float2 velocityVec, float2 position)
{

	float2 newTexCoords = texCoord * float2(screenWidth/20, screenHeight/20);
	
	int2 v = velocityTexture[texCoord];

	//return float4(abs(v.x), abs(v.y), 0.0f, 1.0f);
	if (length(v) <= (0.01f + 0.5f))
	{
		return albedoTexture.Sample(SamplePointClamp, texCoord);
	}	

	//float2 tVelocity = float2(velocityTexture[position].rg);
	//return float4(tVelocity.x, tVelocity.y, 0.0f, 1.0f);

	float weight = 1.0f / length(velocityTexture[position.rg]);
	float4 sum = albedoTexture.Sample(SamplePointClamp, texCoord) * weight;

	float t	 = 0.0f;
	float2 y = 0.0f;
	float zX = 0.0f;
	float zY = 0.0f;
	float f	 = 0.0f;
	float b	 = 0.0f;
	float a  = 0.0f;

	float sampleAmount = 6.0f;
	float j = 0.0f;

	int loopTimes = int(ceil((sampleAmount - 1) / 2.0f));
	[unroll(6)]
	for (int i = 0; i < loopTimes; ++i)
	{
		t = lerp(-1.0f, 1.0f, (i + j + 1.0) / (sampleAmount + 1.0f));
		
		y = round(position + v*t + 0.5f);
		
		zX = positionTexture[position].z;
		zY = positionTexture[y.rg].z;
		
		f = softDepthCompare(zX, zY);
		b = softDepthCompare(zY, zX);
		
		float2 velY = velocityTexture[y.rg];
		float2 velX = velocityTexture[position.rg];
		
		a = f * cone(y, position, velY) 
			+ b*cone(position, y, velX) +
			cylinder(y, position, velY) * cylinder(position, y, velX) * 2.0f;
		
		weight += a; sum += a *albedoTexture[y];
	}

	return sum / weight;
}

float4 DeferredLightingReconstructPixelShader(PixelInputType input) : SV_TARGET
{ 
	float2 texCoord = input.position / float2(screenWidth, screenHeight);

	float4 albedo = albedoTexture.Sample(SamplePointClamp, texCoord);
	float3 position = positionTexture.Sample(SamplePointClamp, texCoord);
	float3 normal = normalTexture.Sample(SamplePointClamp, texCoord);
	float occlusion = occlusionTexture.Sample(SamplePointClamp, texCoord);


	float4 rec = Reconstruct(texCoord, position.z, input.position.xy);
	// Calculate the world space position by multiplying by the inverse view matrix
	// position is in view space
	// Multiplied by the inverse view space will take it to world space
	float4 worldSpacePosition = mul(float4(position.xyz, 1.0f), viewMatrixInversed);


	// Motion blur
	//float velocityScale = framerate / 60.0f;
	//float2 blurVec = float2(velocityTexture.Sample(SamplePointClamp, texCoord).rg) * velocityScale;
	//float4 result = albedoTexture.Sample(SamplePointClamp, texCoord);
	//
	//int amountOfSamples = 6;
	//for (int i = 1; i < amountOfSamples; ++i)
	//{
	//	float2 offset = blurVec * (float(i) / float(amountOfSamples - 1) - 0.5);
	//	result += albedoTexture.Sample(SamplePointClamp, texCoord + offset);
	//}
	//
	//result /= float(amountOfSamples);
	
	
	// Transform the world space position to light projection space
	float4 lightViewPosition = mul(mul(float4(worldSpacePosition.xyz, 1.0f), lightViewMatrix), lightProjectionMatrix);
	return rec;
	float shadowImpact = ShadowMappingPCF(lightViewPosition);
	//return float4(blurVec.xy * 1., 0.0f, 1.0f);
	return PerformLightingDeferred((float3)position, (float3)normal, rec, 1.0f, occlusion) * shadowImpact;

}