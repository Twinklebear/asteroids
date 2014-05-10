#include <SDL.h>
#include <entityx/entityx.h>
#include "components/controllable.h"
#include "events/input_event.h"
#include "systems/input_system.h"

void InputSystem::update(entityx::ptr<entityx::EntityManager> es,
	entityx::ptr<entityx::EventManager> events, double dt)
{
	SDL_Event e;
	while (SDL_PollEvent(&e)){
		events->emit<InputEvent>(e);
		for (auto entity : es->entities_with_components<Controllable>()){
			entityx::ptr<Controllable> cont = entity.component<Controllable>();
			if (cont->enabled){
				cont->control(entity, e);
			}
		}
	}
}

