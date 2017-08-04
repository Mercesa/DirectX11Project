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

	tang = (float)tanf(ANG2RAD * angle * 0.5f);

	nh = tang*nearD;
	nw = nh * ratio;

	fh = tang*farD;
	fw = fh * ratio;
}


void FrustumG::SetCamDef(glm::vec3 p, glm::vec3 l, glm::vec3 u)
{
	glm::vec3 dir, nc, fc, X, Y, Z;

	Z = p - l;
	Z= glm::normalize(Z);

	X = glm::cross(u, Z);

	Y = glm::cross(Z, X);

	nc = p - Z * nearD;
	fc = p - Z * farD;


	// Near plane points
	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;

	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	// Far plane points
	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;

	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;


	planes[TOP].Set3Points(ntr, ntl, ftl);
	planes[BOTTOM].Set3Points(nbl, nbr, fbr);
	planes[LEFT].Set3Points(ntl, nbl, fbl);
	planes[RIGHT].Set3Points(nbr, ntr, fbr);
	planes[NEARP].Set3Points(ntl, ntr, nbr);
	planes[FARP].Set3Points(ftr, ftl, fbl);
}