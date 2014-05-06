#include <random>
#include <tuple>
#include <ctime>
#include <entityx/entityx.h>
#include <entityx/deps/Dependencies.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "util.h"
#include "interleavedbuffer.h"
#include "systems/movement_system.h"
#include "systems/asteroid_system.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/appearance.h"
#include "level.h"

Level::~Level(){
	glDeleteProgram(shader_program);
}
void Level::configure(){
	system_manager->add<MovementSystem>();
	system_manager->add<AsteroidSystem>(4);
	system_manager->add<entityx::deps::Dependency<Asteroid, Position, Velocity>>();
	shader_program = util::load_program({std::make_tuple(GL_VERTEX_SHADER, "../res/vertex.glsl"),
		std::make_tuple(GL_FRAGMENT_SHADER, "../res/fragment.glsl")});
	viewing = InterleavedBuffer<Layout::PACKED, glm::mat4>{2, GL_UNIFORM_BUFFER, GL_STATIC_DRAW};
	assert(shader_program != -1);
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
	viewing.map(GL_WRITE_ONLY);
	viewing.write<0>(0) = glm::lookAt(glm::vec3{0.f, 0.f, 5.f}, glm::vec3{0.f, 0.f, 0.f},
		glm::vec3{0.f, 1.f, 0.f});
	viewing.write<0>(1) = glm::perspective(util::deg_to_rad(75), 640.f/480, 1.f, 100.f);
	viewing.unmap();
	GLuint viewing_block = glGetUniformBlockIndex(shader_program, "Viewing");
	glUniformBlockBinding(shader_program, viewing_block, 0);
	viewing.bind_base(0);
	glUseProgram(shader_program);
}
void Level::update(double dt){
	system_manager->update<MovementSystem>(dt);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	system_manager->update<AsteroidSystem>(dt);
}

