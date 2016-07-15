#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Asset.hpp"
#include "Camera.hpp"
#include "Projectile.hpp"
#include <vector>

namespace GameState {
	extern glm::vec2 windowSize;
	extern glm::vec2 screenMousePosition;
	extern glm::vec2 worldMousePosition;
	extern double deltaTime;
	extern double timeElapsed;
	extern float zoom;
	
	extern std::vector<Projectile> projectiles;
	extern Asset asset;
	extern Camera camera;
}

#endif
