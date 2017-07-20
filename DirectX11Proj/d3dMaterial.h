#pragma once

#include <memory>

class d3dTexture;

class d3dMaterial
{
public:
	d3dMaterial();
	~d3dMaterial();

	std::unique_ptr<d3dTexture> mpDiffuse;
	std::unique_ptr<d3dTexture> mpSpecular;
	std::unique_ptr<d3dTexture> mpNormal;
};

