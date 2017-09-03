#include "FrustumG.h"

#define ANG2RAD 3.14159265358979323846/180.0


FrustumG::FrustumG()
{
}


FrustumG::~FrustumG()
{
}


void FrustumG::SetCamInternals(float angle, float ratio, float nearD, float farD)
{
	this->angle = angle;
	this->ratio = ratio;
	this->nearD = nearD;
	this->farD = farD;

	// Calculate the near and far plane width/height
	tang = (float)tan(angle * 0.5f);
	nh = tang*nearD *2.0f;
	nw = nh * ratio;

	fh = tang*farD * 2.0f;
	fw = fh * ratio;
}


void FrustumG::SetCamDef(glm::vec3 p, glm::vec3 l, glm::vec3 u)
{
	glm::vec3 nc, fc, X, Y, Z;

	// We need l2 assuming l is only a forward vector, not the actual vector
	glm::vec3 l2 = p + l;
	Z = p - l2;


	// Create X Y Z axis with cross products
	Z= glm::normalize(Z);

	X = glm::cross(u, Z);
	X = normalize(X);
	Y = glm::cross(Z, X);

	// Get the near and far plane
	nc = p - Z * nearD;
	fc = p - Z * farD;

	// near plane points

	// precalculate some values
	glm::vec3 yNH = Y*nh;
	glm::vec3 xNW = X*nw;
	glm::vec3 yFH = Y*fh;
	glm::vec3 xFW = X*fw;

	ntl = nc + yNH - xNW;
	ntr = nc + yNH + xNW;

	nbl = nc - yNH - xNW;
	nbr = nc - yNH + xNW;

	// Far plane points
	ftl = fc + yFH - xFW;
	ftr = fc + yFH + xFW;
			   		 
	fbl = fc - yFH - xFW;
	fbr = fc - yFH + xFW;


	planes[TOP].Set3Points(ntr, ntl, ftl);
	planes[BOTTOM].Set3Points(nbl, nbr, fbr);
	planes[LEFT].Set3Points(ntl, nbl, fbl);
	planes[RIGHT].Set3Points(nbr, ntr, fbr);
	planes[NEARP].Set3Points(ntl, ntr, nbr);
	planes[FARP].Set3Points(ftr, ftl, fbl);
}