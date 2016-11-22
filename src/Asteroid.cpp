#include "Asteroid.hpp"
#include <iostream>
#include "GameState.hpp"

Asteroid::Asteroid(const Asset::Texture& texture, glm::dvec2 pos, float rotation, float rotationSpeed, glm::dvec2 speed) : SolidObject(glm::dvec2(0), pos, rotation, speed) {
	SetOwner(GameState::player->GetId());
	isdead = false;
	m_sprite.SetTexture(texture);
	m_size = texture.size;
	m_type = object_type::asteroid;
	
	m_rotation_speed = rotationSpeed;
}

void Asteroid::Destroy() {
	isdead = true;
}

void Asteroid::Draw() {
	m_sprite.DrawSprite(m_size, m_position, m_rotation);
}

void Asteroid::Update() {
	if(m_rotation > 0) {
		//m_rotation += 0.1;
		
		this->SetSpeed(glm::vec2(5, 15));
	}
}

Asteroid::Asteroid(const Asteroid& c) {
	*this = c;
}

Sprite* Asteroid::GetSprite() {
	return &m_sprite;
}
