#ifndef VELOCITY_H
#define VELOCITY_H

#include <glm/glm.hpp>
#include <entityx/entityx.h>

struct Velocity : entityx::Component<Velocity> {
	glm::vec2 vel;

	Velocity(const glm::vec2 &vel = glm::vec2{0.f, 0.f}) : vel(vel) {}
};

#endif

