#ifndef ASTEROID_SYSTEM_H
#define ASTEROID_SYSTEM_H

#include <entityx/entityx.h>
#include "renderbatch.h"
#include "model.h"

class AsteroidSystem : public entityx::System<AsteroidSystem> {
	RenderBatch render_batch;

public:
	AsteroidSystem();
	void update(entityx::ptr<entityx::EntityManager> es,
		entityx::ptr<entityx::EventManager> events, double dt) override;
};

#endif

