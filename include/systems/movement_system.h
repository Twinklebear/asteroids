#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

#include <entityx/entityx.h>

struct MovementSystem : public entityx::System<MovementSystem> {
	void update(entityx::ptr<entityx::EntityManager> es,
		entityx::ptr<entityx::EventManager> events, double dt) override;
};

#endif

