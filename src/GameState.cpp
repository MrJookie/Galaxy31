#include "GameState.hpp"

namespace GameState {
	glm::vec2 windowSize;
	glm::vec2 screenMousePosition;
	glm::vec2 worldMousePosition;
	double deltaTime;
	double timeElapsed;
	float zoom;
	
	Asset asset;
	Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
}
