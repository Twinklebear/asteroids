#include <iostream>
#include <SDL.h>
#include <entityx/entityx.h>

struct Position : entityx::Component<Position> {
	float x, y;
	
	Position(float x, float y) : x(x), y(y) {}
};

int main(int argc, char **argv){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
		return 1;
	}
	entityx::ptr<entityx::EventManager> events(new entityx::EventManager());
	entityx::ptr<entityx::EntityManager> entities(new entityx::EntityManager(events));
	entityx::Entity entity = entities->create();
	entity.assign<Position>(1.f, 2.f);

	entityx::ptr<Position> p = entity.component<Position>();
	std::cout << "entity has pos {" << p->x << ", " << p->y << "}\n";

	SDL_Quit();
	return 0;
}


