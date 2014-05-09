#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <entityx/entityx.h>

struct InputSystem : public entityx::System<InputSystem> {
	void update(entityx::ptr<entityx::EntityManager> es,
		entityx::ptr<entityx::EventManager> events, double dt) override;
};

#endif

