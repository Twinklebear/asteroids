#include <SDL.h>
#include <entityx/entityx.h>
#include "components/velocity.h"
#include "components/controllable.h"

Controllable::Controllable(bool enabled) : enabled(enabled) {}
void Controllable::control(entityx::Entity &entity, const SDL_Event &event){
	//Just some basic keyboard control for now
	entityx::ptr<Velocity> vel = entity.component<Velocity>();
	if (event.type == SDL_KEYDOWN){
		switch (event.key.keysym.sym){
			case SDLK_w:
				vel->vel.y = 0.5f;
				break;
			case SDLK_s:
				vel->vel.y = -0.5f;
				break;
			case SDLK_d:
				vel->vel.x = 0.5f;
				break;
			case SDLK_a:
				vel->vel.x = -0.5f;
				break;
		}
	}
	if (event.type == SDL_KEYUP){
		switch (event.key.keysym.sym){
			case SDLK_w:
			case SDLK_s:
				vel->vel.y = 0;
				break;
			case SDLK_d:
			case SDLK_a:
				vel->vel.x = 0;
				break;
		}
	}
}

