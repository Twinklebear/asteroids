#include <random>
#include <tuple>
#include <ctime>
#include <entityx/entityx.h>
#include <entityx/deps/Dependencies.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "util.h"
#include "systems/movement_system.h"
#include "systems/asteroid_system.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/appearance.h"
#include "level.h"

void Level::configure(){
	system_manager->add<MovementSystem>();
	system_manager->add<AsteroidSystem>(4);
	system_manager->add<entityx::deps::Dependency<Asteroid, Position, Velocity>>();
}
void Level::initialize(){
	std::mt19937 mt_rand;
	mt_rand.seed(std::time(0));
	for (int i = 0; i < 4; ++i){
		entityx::Entity e = entity_manager->create();
		e.assign<Position>(glm::vec2{static_cast<float>(mt_rand() % 8) - 4,
			static_cast<float>(mt_rand() % 8) - 4});
		e.assign<Velocity>(0.25f * glm::vec2{static_cast<float>(mt_rand() % 4) - 2,
			static_cast<float>(mt_rand() % 4) - 2});
		e.assign<Asteroid>();
	}
}
void Level::update(double dt){
	system_manager->update<MovementSystem>(dt);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	system_manager->update<AsteroidSystem>(dt);
}

