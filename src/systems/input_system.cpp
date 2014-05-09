#include <SDL.h>
#include <entityx/entityx.h>
#include "events/input_event.h"
#include "systems/input_system.h"

void InputSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt)
{
	SDL_Event e;
	while (SDL_PollEvent(&e)){
		events->emit<InputEvent>(e);
	}
}

