#ifndef LEVEL_H
#define LEVEL_H

#include <entityx/entityx.h>

class Level : public entityx::Manager {
protected:
	void configure() override;
	void initialize() override;
	void update(double dt) override;
};

#endif

