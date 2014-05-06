#include <vector>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <entityx/entityx.h>
#include "renderbatch.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/appearance.h"
#include "systems/asteroid_system.h"

AsteroidSystem::AsteroidSystem() : render_batch(4, Model{"../res/polyhedron.obj"}){
	std::cout << "Asteroid system running\n";
}
void AsteroidSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt){
	/*
	size_t i = 0;
	std::vector<std::tuple<size_t, glm::mat4>> updates;
	for (auto entity : es->entities_with_components<Asteroid>()){
		entityx::ptr<Position> pos = entity.component<Position>();
		updates.push_back(std::make_tuple(i, glm::translate(glm::vec3{pos->pos})
			* glm::scale(glm::vec3{0.5f, 0.5f, 0.5f})));
	}
	*/
}

