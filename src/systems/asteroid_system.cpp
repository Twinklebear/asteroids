#include <random>
#include <ctime>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <entityx/entityx.h>
#include "util.h"
#include "renderbatch.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/appearance.h"
#include "systems/asteroid_system.h"

AsteroidSystem::AsteroidSystem(size_t n)
	: render_batch(n, Model{util::get_resource_path() + "suzanne.obj"}){
	//Everything's just gonna use the same program
	render_batch.set_attrib_indices(std::array<int, 2>{3, 7});
}
void AsteroidSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt){
	std::vector<std::tuple<glm::mat4, int>> updates;
	std::mt19937 gen{std::time(0)};
	std::uniform_int_distribution<int> color{0, 2};
	size_t i = 0;
	for (auto entity : es->entities_with_components<Asteroid>()){
		entityx::ptr<Position> pos = entity.component<Position>();
		updates.push_back(std::make_tuple(glm::translate(glm::vec3{pos->pos.x, pos->pos.y, 1.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 0.5f}), color(gen)));
		++i;
	}
	if (i > render_batch.batch_size()){
		render_batch.resize(i);
	}
	render_batch.update(updates);
	render_batch.render();
}

