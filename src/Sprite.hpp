#ifndef SPRITE_HPP
#define SPRITE_HPP

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

#include "GameState.hpp"

class Sprite {
	public:
		Sprite();
		~Sprite();
		Sprite& operator=(Sprite &&);
		Sprite& operator=(const Sprite& o) = default;
		
		void Draw();
		
		void SetTexture(GLuint textureID);
		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		
		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;

	private:
		glm::mat4 m_modelMat;
		
		GLuint m_vao, m_vbo[2], m_ebo;
		GLuint m_texture;
		
		glm::vec2 m_size;
		glm::vec2 m_position;
		
		float m_rotation;
};

#endif
