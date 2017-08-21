
struct Light
{
	float3 position;
	float padL0;

	float3 colour;
	float padL1;

	float3 specularColor;
	int lightType;
};

cbuffer MatrixBuffer : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix viewMatrixInversed;
	matrix projectionMatrixInverse;
	matrix projViewMatrix;
	matrix prevProjViewMatrix;

	float3 gEyePos;
	float1 pad0;
};

// integers are 32 bit in HLSL
cbuffer MaterialBuffer : register(b1)
{
	int hasDiffuse;
	int hasSpecular;
	int hasNormal;
	int padding0;
};


cbuffer LightBuffer : register(b2)
{
	int amountOfLights;
	int padLB01;
	int padLB02;
	int padLB03;

	Light directionalLight;

	Light arr[16];
};


cbuffer LightMatrixBuffer : register (b3)
{
	float shadowMapWidth;
	float shadowMapHeight;
	float lmbPad01;
	float lmbPad02;

	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
	matrix lightProjectionViewMatrix;

	float3 kernelSamples[64];
}


cbuffer PerObjectBuffer : register(b4)
{
	matrix worldMatrix;
	matrix prevWorldMatrix;
}


cbuffer BlurBuffer : register(b5)
{
	int blurHorizontal;
	int pad0B;
	int pad1B;
	int pad2B;
}


cbuffer GenericAttributesBuffer : register(b6)
{
	float screenWidth;			// 4 bytes
	float screenHeight;			// 8 bytes
	float nearPlaneDistance;	// 12 bytes
	float farPlaneDistance;		// 16 bytes	
	
	float totalApplicationTime; // 20 bytes
	float deltaTime;			// 24 bytes
	float framerate;			// 28 bytes
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Transform from 0 1 to -1.0f 2.0f;
	float3 normalT = normalMapSample * 2.0f - 1.0f;

	float3 N = unitNormalW;
	// Calculate tangent with gramm-schmidt orthonormalization 
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);
	
	// Transform from tangent space to world space
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

float DoAttenuation(Light light, float d)
{
	// Calculates attenuation, currently fixed attenuation amounts
	return 1.0f / (1.0f + 0.002* d + (0.02*d *d));
}

float4 DoDiffuse(Light light, float3 L, float3 N)
{
	// Do basic dot product with light vector and normal
	return max(dot(N, L), 0.0f);
}

float4 DoSpecular(Light light, float3 V, float3 L, float3 N)
{
	// Inverse light vector, reflect it
	float3 reflV = reflect(-L, N);

	// Calculate the specular intensity
	float spec = pow(max(dot(V, reflV), 0.0f), 8.0f);

	return float4(spec, spec, spec, 1.0);
}

float4 DoPointLight(Light light, float3 V, float3 P, float3 N, float4 diffTextureColor, float specTextureColor)
{
	// Calculate light vector
	float3 L = light.position - P;
	float distance = length(L);

	// Normalize vector
	L = L / distance;

	// texture * normal dot product * light colour * attenuation
	float4 ambientCol = diffTextureColor	*	float4(light.colour.rgb, 0.0f);
	float4 diffuseCol = diffTextureColor   *	DoDiffuse(light, L, N)		* float4(light.colour.rgb, 1.0f);
	float4 specularCol = specTextureColor * 1.0f *  DoSpecular(light, V, L, N) ;

	float attenuation = DoAttenuation(light, distance);

	ambientCol *= attenuation;
	diffuseCol *= attenuation;
	specularCol *= attenuation;

	float4 combined = ambientCol + diffuseCol + specularCol;


	return combined;
}

float4 DoDirectionalLight(Light light, float3 V, float3 P, float3 N, float4 diffTextureColor, float specTextureColor, float aOcclusion)
{
	// Calculate light vector
	float3 L = normalize(-light.position.xyz);

	// texture * normal dot product * light colour * attenuation
	float4 ambientCol = diffTextureColor   *	float4(light.colour.rgb * aOcclusion * 0.6, 1.0);
	float4 diffuseCol = diffTextureColor   *	DoDiffuse(light, L, N)		* float4(light.colour.rgb, 1.0f);
	float4 specularCol = specTextureColor * 10.0f *  DoSpecular(light, V, L, N)  * diffTextureColor;


	float4 combined = ambientCol + diffuseCol  + specularCol;

	return combined;
}

float4 PerformLighting(float3 aFragPosition, float3 aNormal, float4 aDiffMapSample, float aSpecMapSample, float aOcclusion)
{
	float4 tResultCol = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Loop through all the lights(point lights in this case)
	float3 eyeDir = normalize(gEyePos - aFragPosition);

	for (float i = 0; i < amountOfLights; ++i)
	{
		// From frag position to eye 	
		tResultCol += DoPointLight(arr[i], eyeDir, aFragPosition, normalize(aNormal), aDiffMapSample, aSpecMapSample);
	}
	tResultCol += DoDirectionalLight(directionalLight, eyeDir, aFragPosition, normalize(aNormal), aDiffMapSample, aSpecMapSample, aOcclusion);

	return tResultCol;
}

float4 DoDirectionalLightDeferred(Light light, float3 V, float3 P, float3 N, float4 diffTextureColor, float specTextureColor, float aOcclusion)
{
	// Calculate light vector
	float3 L = normalize(-light.position.xyz);

	// Put our light direction vector to viewspace
	L = (float3)mul(float4(L.rgb, 0.0), viewMatrix);

	// texture * normal dot product * light colour * attenuation
	float4 ambientCol = diffTextureColor   *	float4(light.colour.rgb * 0.6f * aOcclusion * 2.0f, 1.0);
	float4 diffuseCol = diffTextureColor   *	DoDiffuse(light, L, N)		* float4(light.colour.rgb, 1.0f);
	float4 specularCol = specTextureColor  *	DoSpecular(light, V, L, N)  * float4(light.colour.rgb, 1.0f);

	float4 combined = ambientCol + diffuseCol;

	return combined;
}

float4 PerformLightingDeferred(float3 aFragPosition, float3 aNormal, float4 aDiffMapSample, float aSpecMapSample, float aOcclusion)
{
	float4 tResultCol = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Loop through all the lights(point lights in this case)
	float3 eyeDir = normalize(-aFragPosition);

	for (float i = 0; i < amountOfLights; ++i)
	{
		// From frag position to eye 	
		tResultCol += DoPointLight(arr[i], eyeDir, aFragPosition, normalize(aNormal), aDiffMapSample, aSpecMapSample);
	}
	tResultCol += DoDirectionalLightDeferred(directionalLight, eyeDir, aFragPosition, normalize(aNormal), aDiffMapSample, aSpecMapSample, aOcclusion);

	return tResultCol;
}
