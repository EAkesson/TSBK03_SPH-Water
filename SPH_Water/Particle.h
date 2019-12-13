#pragma once

#include <glm/glm.hpp>
using namespace glm;

struct Particle
{
	glm::vec3 pos, speed, force, force2;
	float density, pressure;


	
	unsigned char r, g, b, a; // Color
	float size, angle, weight;	
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f
	bool controllable;

	bool operator<(const Particle& that) const;
};

