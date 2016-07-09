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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Sprite {
	public:
		Sprite();
		Sprite(std::string imageFile);
		~Sprite();
		
		void DrawSprite(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
		
		int m_width;
		int m_height;


	private:
		GLuint textureFromFile(std::string imageFile);
		
		GLuint vao, vbo[2], ebo;
		GLuint texture;
};
