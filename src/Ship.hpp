#ifndef SHIP_HPP
#define SHIP_HPP

#include "Asset.hpp"
#include "GameState.hpp"
#include "Object.hpp"
#include "Sprite.hpp"

class Ship : public Object {
	public:
		Ship(glm::vec2 position, float rotation, glm::vec2 speed, float acceleration, int chassiId, std::string chassiName, Asset::Texture chassiTexture, Asset::Texture chassiSkin, float chassiMass, float chassiArmor);
		~Ship();
		
		void Process();
		void Draw();
		
	private:
		int m_chassi_id;
		std::string m_chassi_name;
		GLuint m_chassi_texture;
		GLuint m_chassi_texture_skin;
		float m_chassi_mass;
		float m_chassi_armor;
		
		Sprite m_chassi_sprite;
		Sprite m_engine_propulsion;

};

#endif
