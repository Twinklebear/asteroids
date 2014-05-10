#ifndef CONTROLLABLE_H
#define CONTROLLABLE_H

#include <SDL.h>
#include <entityx/entityx.h>

//TODO: Configurable acceleration for the component?
struct Controllable : entityx::Component<Controllable> {
	bool enabled;

	Controllable(bool enabled = true);
	void control(entityx::Entity &entity, const SDL_Event &event);
};

#endif

