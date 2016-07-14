#include "Ship.hpp"

Ship::Ship(glm::vec2 position, float rotation, glm::vec2 speed, float acceleration, int chassiId, std::string chassiName, Asset::Texture chassiTexture, Asset::Texture chassiSkin, float chassiMass, float chassiArmor) {
	m_size = chassiTexture.size;
	m_position = position;
	m_rotation = rotation;

	m_speed = speed;
	m_acceleration = acceleration;
	
	m_chassi_id = chassiId;
	m_chassi_name = chassiName;
	m_chassi_texture = chassiTexture.id;
	m_chassi_texture_skin = chassiSkin.id;
	m_chassi_mass = chassiMass;
	m_chassi_armor = chassiArmor;
	
	m_chassi_sprite.SetSize(m_size);
	m_chassi_sprite.SetPosition(m_position);
	m_chassi_sprite.SetRotation(m_rotation);
	m_chassi_sprite.SetTexture(m_chassi_texture);
}

Ship::~Ship() {}

void Ship::Draw() {
	m_chassi_sprite.DrawSprite(m_size, m_position, m_rotation); //chassi only
	//draw loaded mountables within propulsion on engine mountable
	
	const float a = 0.4;
	if(m_last_acceleration > 0.1) {
        m_engine_propulsion.SetSize(glm::vec2(GetSize().x*0.5, GetSize().y * 0.6 * std::max(0.01f, m_last_acceleration*a)) );
        float theta = (m_chassi_sprite.GetRotation() + 90) * 3.141592 / 180.0;
        const glm::vec2 &pos = m_chassi_sprite.GetPosition();
        const glm::vec2 &size = m_chassi_sprite.GetSize();
        const glm::vec2 &psize = m_engine_propulsion.GetSize();
        const float A = size.y * 0.5 + psize.y*0.5;
        m_engine_propulsion.SetPosition(glm::vec2(
            pos.x + A*cos(theta) + psize.x*0.5,
            pos.y + size.y * 0.5 + A*sin(theta) - psize.y*0.5
        ));
        m_engine_propulsion.SetRotation( m_chassi_sprite.GetRotation() );
        m_engine_propulsion.SetTexture(GameState::asset.GetTexture("Assets/Textures/propulsion.png").id);
        m_engine_propulsion.DrawSprite(m_engine_propulsion.GetSize(), glm::vec2(this->GetPosition().x - m_engine_propulsion.GetSize().x * 0.5, this->GetPosition().y - m_engine_propulsion.GetSize().y * 0.5), this->GetRotation());
    }
}

void Ship::Process() {
	glm::vec2 pos_to_mouse_vector = glm::vec2( GameState::worldMousePosition.x - this->GetPosition().x, GameState::worldMousePosition.y - this->GetPosition().y );
	glm::vec2 direction = glm::normalize(pos_to_mouse_vector);
	float distance = glm::length(pos_to_mouse_vector);
	float angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), direction ) );
	this->SetRotation( angle + 90 );

	if(distance > (this->GetSize().y * 0.5)) {
		this->Accelerate(direction * distance * this->GetAcceleration());
	}
	
    this->SetPosition(glm::vec2(this->GetPosition().x + GameState::deltaTime * this->GetSpeed().x, this->GetPosition().y + GameState::deltaTime * this->GetSpeed().y));
}
