
Texture2D<int2> gInputA;
RWTexture2D<int2> gOutput;

// This function returns the greater vector
int2 GetMaxVector(int2 a, int2 b)
{
	float lA = length(a);
	float lB = length(b);

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

	for (int i = -1; i < 1; ++i)
	{
		for (int j = -1; j < 1; ++j)
		{
			currMax = GetMaxVector(currMax, gInputA[DTid.xy + int2(i,j)]);
		}
	}

	gOutput[DTid.xy] = currMax;
}