
Texture2D<int2> gInputA;
RWTexture2D<int2> gOutput;

int2 GetMaxVector(int2 a, int2 b)
{
	float lA = length(float2(a));
	float lB = length(float2(b));

	int2 tA = a;
	int2 tB = b;
	if (lA >= lB)
	{
		return tA;
	}
	return tB;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
	int2 currMax = int2(0, 0);

	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 20; ++j)
		{
			currMax = GetMaxVector(currMax, gInputA[DTid.xy * int2(20,20) + int2(i,j)]);
		}
	}

	gOutput[DTid.xy] = currMax;
}