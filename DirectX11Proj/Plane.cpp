#include "Plane.h"

Plane::Plane() :
	normal(glm::vec3(0.0f)),
	point(glm::vec3(0.0f)),
	d(0.0f){}

Plane::Plane(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
{
	Set3Points(v1, v2, v3);
}


Plane::~Plane()
{
}

void Plane::Set3Points(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
{
	glm::vec3 aux1, aux2;

	// Create two vectors which are perpendicular
	aux1 = v1 - v2;
	aux2 = v3 - v2;

	// Create normal using the two vectors that align on the same plane
	normal = cross(aux2, aux1);
	normal = normalize(normal);
	
	point = v2;

	// It is possible to figure out the distance from the plane by projecting with the dot product
	d = -(glm::dot(normal, point));
}

float Plane::Distance(glm::vec3& p)
{
	return (d + dot(normal, p));
}