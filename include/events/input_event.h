#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

#include <SDL.h>
#include <entityx/entityx.h>

struct InputEvent : public entityx::Event<InputEvent> {
	SDL_Event event;

	InputEvent(const SDL_Event &e) : event(e) {}
};

#endif

