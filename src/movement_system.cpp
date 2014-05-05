#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <entityx/entityx.h>
#include "components.h"
#include "movement_system.h"

void MovementSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt){
	for (auto entity : es->entities_with_components<Position, Velocity>()){
		entityx::ptr<Position> pos = entity.component<Position>();
		entityx::ptr<Velocity> vel = entity.component<Velocity>();
		pos->pos += vel->vel * static_cast<float>(dt);
	}
}

