#pragma once

#include <d3d11.h>	
#include <wrl.h>

class d3dDXGIManager
{
public:
	d3dDXGIManager();
	~d3dDXGIManager();

	bool Create(int aWidth, int aHeight, int &numerator, int &denominator);
	void GetVideoCardInfo(char* cardName, int& memory);

	int m_videoCardMemory;
	char m_videoCardDescription[128];

	IDXGIFactory* const GetFactory() { return mFactory.Get(); }

private:
	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;


};

