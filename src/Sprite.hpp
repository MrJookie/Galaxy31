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
		
		void DrawSprite(GLuint shader, glm::mat4 view, glm::mat4 projection);
		
		
		void SetSize(int sizeX, int sizeY);
		void SetPosition(int posX, int posY);
		void SetRotation(float rotation);
		
		glm::vec2 GetSize();
		glm::vec2 GetPosition();
		float GetRotation();
		


	private:
		GLuint textureFromFile(std::string imageFile);
		
		glm::mat4 m_modelMat;
		
		GLuint m_vao, m_vbo[2], m_ebo;
		GLuint m_texture;
		
		glm::vec2 m_size;
		glm::vec2 m_position;
		
		float m_rotation;
};
