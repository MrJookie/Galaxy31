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

#include "Camera.hpp"

#include "Sprite.hpp"

class Ship {
	public:
		Ship();
		Ship(std::string);
		~Ship();

		void Accelerate(glm::vec2 acceleration);
		
		void SetSize(int sizeX, int sizeY);
		void SetPosition(float posX, float posY);
		void SetRotation(float rotation);
		void SetSpeed(glm::vec2 speed);
		void SetSkin(std::string imageFile);
		
		glm::vec2 GetSize();
		glm::vec2 GetPosition();
		float GetRotation();
		glm::vec2 GetSpeed();
		
		void SetSpriteShader(unsigned int spriteShader);
		
		void Draw(Camera &cam);
		
		void Process(double delta_time);
		
	private:
		GLuint m_sprite_shader;
		float m_last_acceleration;
		Sprite m_sprite;
		glm::vec2 m_speed;
		
		static bool initialized_statics;
		static Sprite m_engine_propulsion;
		
		void init();
};
#endif
