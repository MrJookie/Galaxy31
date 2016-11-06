#include "Projectile.hpp"
#include <iostream>
#include "GameState.hpp"

Projectile::Projectile(const Asset::Texture& texture, glm::dvec2 pos, glm::dvec2 speed) : SolidObject(glm::dvec2(0), pos, 0, speed) {
	SetOwner(GameState::player->GetId());
	isdead = false;
	m_sprite.SetTexture(texture);
	m_size = texture.size;
	timer = 20;
	m_type = object_type::projectile;
	m_last_position = glm::vec2(pos);
	
	//std::cout << pos.x << ", " << pos.y << std::endl;
	//std::cout << m_last_position.x << ", " << m_last_position.y << std::endl;
	
	
}

void Projectile::Destroy() {
	isdead = true;
}

void Projectile::Draw() {
	m_sprite.DrawSprite(m_size, m_position, m_rotation);
}

void Projectile::Update() {
	timer -= GameState::deltaTime;
	//std::cout << 
	
	float rayLen = glm::length(this->GetSpeed()) * 0.05;
	glm::vec2 projectileDirection = glm::normalize(glm::dvec2(m_position.x - m_last_position.x, m_position.y - m_last_position.y));
	
	std::vector<glm::vec2> rayLine;
	rayLine.push_back(m_position);
	rayLine.push_back(glm::vec2(m_position.x + projectileDirection.x * rayLen, m_position.y + projectileDirection.y * rayLen));
	
	UpdateProjectileRay(rayLine);
	
	RenderProjectileRay(rayLine);
	
	//std::cout << "Projectile: " << m_position.x << ", " << m_position.y << std::endl;
	
	m_last_position = m_position;
}

Projectile::Projectile(const Projectile& c) {
	*this = c;
}

bool Projectile::IsDead() {
	return isdead || timer < 0;
}

Sprite* Projectile::GetSprite() {
	return &m_sprite;
}
