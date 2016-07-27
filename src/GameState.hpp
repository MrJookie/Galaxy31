#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Asset.hpp"
#include "Camera.hpp"
#include "Projectile.hpp"
#include "Object.hpp"
#include "Ship.hpp"
#include <vector>
#include <map>

namespace GameState {
	extern glm::vec2 windowSize;
	extern glm::vec2 screenMousePosition;
	extern glm::vec2 worldMousePosition;
	extern glm::vec2 worldSize;
	extern double deltaTime;
	extern double timeElapsed;
	extern float zoom;
	
	extern int objectsDrawn;
	
	extern std::vector<Projectile> projectiles;
	extern Asset asset;
	extern Camera camera;
	extern Ship *player;
	extern std::map<uint,Ship*> ships;
}

#endif
