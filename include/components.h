#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <glm/glm.hpp>
#include <entityx/entityx.h>

struct Position : entityx::Component<Position> {
	glm::vec2 pos;

	Position(const glm::vec2 &pos = glm::vec2{0.f, 0.f});
};

struct Velocity : entityx::Component<Velocity> {
	glm::vec2 vel;

	Velocity(const glm::vec2 &vel = glm::vec2{0.f, 0.f});
};

#endif

