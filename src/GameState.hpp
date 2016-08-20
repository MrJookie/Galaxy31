#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Asset.hpp"
#include "Camera.hpp"
#include "Projectile.hpp"
#include "Object.hpp"
#include "Ship.hpp"
#include <vector>
#include <utility>
#include <queue>
#include <map>

namespace GameState {
	extern glm::vec2 windowSize;
	extern glm::vec2 screenMousePosition;
	extern glm::vec2 worldMousePosition;
	extern glm::vec2 worldSize;
	extern double deltaTime;
	extern double timeElapsed;
	extern float zoom;
	extern std::string activePage;
	
	extern int objectsDrawn;
	
	extern std::vector<Projectile> projectiles;
	extern ng::GuiEngine gui;
	extern Asset asset;
	extern Camera camera;
	extern Ship* player;
	
	extern std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > ships;
	
	//move to Ship
	extern int account_challenge;
	extern unsigned int user_id;
	extern std::string user_name;
}

#endif
