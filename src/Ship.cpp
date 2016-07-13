#include "Ship.hpp"

Ship::Ship(int id, std::string name, Asset::Texture chassiTexture, Asset::Texture chassiSkin, float mass, float armor) {
	m_chassi.m_id = id;
	m_chassi.m_name = name;
	m_chassi.m_texture = chassiTexture.id;
	m_chassi.m_skin_texture = chassiSkin.id;
	m_chassi.m_mass = mass;
	m_chassi.m_armor = armor;
	
	m_size = chassiTexture.size;
	
	m_sprite.SetSize(m_size);
	m_sprite.SetTexture(m_chassi.m_texture);
	//m_sprite.SetPosition(glm::vec2(100,100));
	//m_sprite.SetRotation(rotation);
}

Ship::~Ship() {}

void Ship::Draw() {
	m_sprite.Draw(); //chassi only
	//draw loaded mountables within propulsion on engine mountable
}

void Ship::Process() {
    this->SetPosition(glm::vec2(this->GetPosition().x + GameState::deltaTime * this->GetSpeed().x, this->GetPosition().y + GameState::deltaTime * this->GetSpeed().y));
}
