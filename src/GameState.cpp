#include "GameState.hpp"

namespace GameState {	
	glm::vec2 windowSize;
	glm::vec2 screenMousePosition;
	glm::vec2 worldMousePosition;
	glm::vec2 worldSize(100000.0f, 100000.0f);
	double deltaTime;
	double timeElapsed;
	float zoom;
	std::string activePage;
	
	int objectsDrawn = 0;
	
	ng::GuiEngine gui;
	Asset asset;
	Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
	std::vector<Projectile> projectiles;
	Ship* player;
	
	std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > ships;
	
	//move to Ship
	int account_challenge;
	unsigned int user_id;
	std::string user_name;
	std::string publicKeyStr;
}
