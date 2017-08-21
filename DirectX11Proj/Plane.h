#pragma once

#include <glm\common.hpp>
#include <glm\gtx\common.hpp>



class Plane
{
public:
	Plane();
	Plane(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
	~Plane();

	void Set3Points(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
	float Distance(glm::vec3& p) const;

	glm::vec3 normal, point;
	float d;


	
};

