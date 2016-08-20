#ifndef SHIP_HPP
#define SHIP_HPP

#include "Asset.hpp"
#include "Object.hpp"
#include "Sprite.hpp"

class Ship : public Object {
	public:
		struct Chassis {
			Chassis();
			Chassis(std::string _name, std::string _texture, std::string _skin);
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
		
		Object* GetObject();
		//move here all account_user info
		
	private:
		Chassis m_chassis;
		float m_rotation_speed_coefficient;
		int m_max_distance_acceleration;
		float m_downshift_coefficient;
		float m_max_speed_coefficient;
		float m_acceleration_speed_coefficient;
		float m_brake_coefficient;
		float m_engine_propulsion_coefficient;
		bool m_stabilizers_on;
		Sprite m_engine_propulsion;
};


#endif
