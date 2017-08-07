#pragma once

#include <glm/common.hpp>
#include <glm/gtx/common.hpp>

#include "Plane.h"

enum sides { TOP, BOTTOM, LEFT, RIGHT, NEARP, FARP };
#include <iostream>
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

	int FrustumG::pointInFrustum(glm::vec3 &p) {

		int result = 1;

		for (int i = 0; i < 6; i++) {
			if (planes[i].Distance(p) < 0)
			{
				return 0;
			}
		}

		return 1;
	}

	int FrustumG::sphereInFrustum(glm::vec4 &p) {

		float distance;
		int result = 1;

		for (int i = 0; i < 6; i++) {
			distance = planes[i].Distance(glm::vec3(p));
			if (distance < -p.w)
				return 0;
			else if (distance < p.w)
				result = 1;
		}
		return(result);
	}

	Plane planes[6];
};



