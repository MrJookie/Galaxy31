#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Asset.hpp"
#include "Camera.hpp"

namespace GameState {
	extern glm::vec2 windowSize;
	extern glm::vec2 screenMousePosition;
	extern glm::vec2 worldMousePosition;
	extern double deltaTime;
	extern double timeElapsed;
	
	extern Asset asset;
	extern Camera camera;
}

#endif
