#ifndef LEVEL_H
#define LEVEL_H

#include <glm/glm.hpp>
#include <entityx/entityx.h>
#include "interleavedbuffer.h"
#include "events/input_event.h"

class Level : public entityx::Manager, public entityx::Receiver<InputEvent> {
	GLint shader_program;
	InterleavedBuffer<Layout::PACKED, glm::mat4> viewing;
	bool quit;
	
public:
	Level();
	~Level();
	void receive(const InputEvent &input);
	bool should_quit();

protected:
	void configure() override;
	void initialize() override;
	void update(double dt) override;
};

#endif

