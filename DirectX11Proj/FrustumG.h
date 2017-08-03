#pragma once

#include <glm/common.hpp>
#include <glm/gtx/common.hpp>

#include "Plane.h"

enum sides { TOP, BOTTOM, LEFT, RIGHT, NEARP, FARP };

class FrustumG
{
public:
	FrustumG();
	~FrustumG();


	float nw, nh, fh, fw;
	float nearD, farD, ratio, angle, tang;
	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;

	void SetCamInternals(float angle, float ratio, float nearD, float farD);

	void SetCamDef(glm::vec3 p, glm::vec3 l, glm::vec3 u);

	Plane planes[6];
};

