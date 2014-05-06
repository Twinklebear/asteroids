#ifndef LEVEL_H
#define LEVEL_H

#include <glm/glm.hpp>
#include <entityx/entityx.h>
#include "interleavedbuffer.h"

class Level : public entityx::Manager {
	GLint shader_program;
	InterleavedBuffer<Layout::PACKED, glm::mat4> viewing;
	
public:
	~Level();

protected:
	void configure() override;
	void initialize() override;
	void update(double dt) override;
};

#endif

