#ifndef SHIP_HPP
#define SHIP_HPP

#include "Asset.hpp"
#include "GameState.hpp"
#include "Object.hpp"
#include "Sprite.hpp"

class Ship : public Object {
	public:
		struct Chassis {
			Chassis(){}
			Chassis(std::string _name, std::string _texture, std::string _skin) {
				name = _name;
				Asset::Texture tex = GameState::asset.GetTexture(_texture);
				texture = tex.id;
				skin = GameState::asset.GetTexture(_skin).id;
				sprite.SetSize(tex.size);
			}
			std::string name;
			GLuint texture;
			GLuint skin;
			float mass;
			float armor;
			Sprite sprite;
		};

		Ship(glm::vec2 position, float rotation, const Chassis& chassis);
		~Ship();
		
		void Process();
		void Process2();
		void Draw();
		void Fire();
		void Stabilizers();
		
	private:
		Chassis m_chassis;
		float m_rotation_speed_coefficient;
		bool m_stabilizers_on;
		Sprite m_engine_propulsion;
};


#endif
