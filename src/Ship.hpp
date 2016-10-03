#ifndef SHIP_HPP
#define SHIP_HPP

#include "Asset.hpp"
#include "Object.hpp"
#include "SolidObject.hpp"
#include "Sprite.hpp"

class Ship : public SolidObject {
	public:
		struct Chassis {
			Chassis();
			Chassis(std::string _name, std::string _texture, std::string _skin);
			std::string name;
			GLuint texture;
			GLuint skin;
			double mass;
			double armor;
			Sprite sprite;
		};

		Ship(glm::dvec2 position, double rotation, const Chassis& chassis);
		~Ship();
		
		void Process();
		//void ProcessOLD();
		void Draw();
		void Fire();
		void Stabilizers();
		Sprite* GetSprite();

		//move here all account_user info
		
	private:
		Chassis m_chassis;
		double m_rotation_speed_coefficient;
		int m_max_distance_acceleration;
		double m_downshift_coefficient;
		double m_max_speed_coefficient;
		double m_acceleration_speed_coefficient;
		double m_brake_coefficient;
		double m_engine_propulsion_coefficient;
		bool m_stabilizers_on;
		Sprite m_engine_propulsion;
};


#endif
