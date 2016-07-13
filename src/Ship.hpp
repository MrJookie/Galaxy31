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

#include "Asset.hpp"
#include "GameState.hpp"
#include "Object.hpp"
#include "Sprite.hpp"

class Ship : public Object {
	struct Chassi {
		int m_id;
		std::string m_name;
		GLuint m_texture;
		GLuint m_skin_texture;
		float m_mass;
		float m_armor;
		//std::vector<char> m_mountables_matrix;
	};
	
	public:
		Ship(int id, std::string name, Asset::Texture chassiTexture, Asset::Texture chassiSkin, float mass, float armor);
		~Ship();
		
		void Process();
		void Draw();
		
	private:
		Chassi m_chassi;
		Sprite m_sprite;

};

#endif
