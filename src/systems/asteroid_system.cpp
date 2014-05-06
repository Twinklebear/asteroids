#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <entityx/entityx.h>
#include "renderbatch.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/appearance.h"
#include "systems/asteroid_system.h"

AsteroidSystem::AsteroidSystem(size_t n) : render_batch(n, Model{"../res/polyhedron.obj"}){
	//Everything's just gonna use the same program
	render_batch.set_attrib_index(3);
}
void AsteroidSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt){
	std::vector<glm::mat4> updates;
	size_t i = 0;
	for (auto entity : es->entities_with_components<Asteroid>()){
		entityx::ptr<Position> pos = entity.component<Position>();
		updates.push_back(glm::translate(glm::vec3{pos->pos.x, pos->pos.y, 1.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 0.5f}));
		++i;
	}
	if (i > render_batch.batch_size()){
		render_batch.resize(i);
	}
	render_batch.update(updates);
	render_batch.render();
}

