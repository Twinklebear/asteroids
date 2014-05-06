#ifndef POSITION_H
#define POSITION_H

#include <glm/glm.hpp>
#include <entityx/entityx.h>

struct Position : entityx::Component<Position> {
	glm::vec2 pos;

	Position(const glm::vec2 &pos = glm::vec2{0.f, 0.f}) : pos(pos) {}
};



#endif

