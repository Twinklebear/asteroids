#include <random>
#include <entityx/entityx.h>
#include <glm/glm.hpp>
#include "systems/movement_system.h"
#include "components/position.h"
#include "components/velocity.h"
#include "level.h"

void Level::configure(){
	system_manager->add<MovementSystem>();
}
void Level::initialize(){
	std::mt19937 mt_rand;
	for (int i = 0; i < 20; ++i){
		entityx::Entity e = entity_manager->create();
		e.assign<Position>(glm::vec2{static_cast<float>(mt_rand() % 8) - 4,
			static_cast<float>(mt_rand() % 8) - 4});
		e.assign<Velocity>(0.25f * glm::vec2{static_cast<float>(mt_rand() % 4) - 2,
			static_cast<float>(mt_rand() % 4) - 2});
	}
}
void Level::update(double dt){
	system_manager->update<MovementSystem>(dt);
}

