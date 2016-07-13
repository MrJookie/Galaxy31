#ifndef SHIP_HPP
#define SHIP_HPP

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
#include "Sprite.hpp"

class Ship {
	public:
		Ship();
		~Ship();
		
		void SetSpriteTexture(GLuint textureID);
		void SetSpriteShader(GLuint spriteShader);
		
		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		void SetSpeed(glm::vec2 speed);
		void SetAcceleration(float acceleration);

		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;
		glm::vec2 GetSpeed() const;
		float GetAcceleration() const;
		
		void Accelerate(glm::vec2 acceleration);
		
		void Draw();
		
		void Process();
		
	private:
		Sprite m_sprite;
		
		GLuint m_sprite_shader;
		float m_acceleration;
		float m_last_acceleration;
		glm::vec2 m_speed;

		//static Sprite m_engine_propulsion;
};

#endif
