#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <entityx/entityx.h>
#include "components.h"

Position::Position(const glm::vec2 &pos) : pos(pos) {}
Velocity::Velocity(const glm::vec2 &vel) : vel(vel) {}

