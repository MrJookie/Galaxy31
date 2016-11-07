#include "Projectile.hpp"
#include <iostream>
#include "GameState.hpp"

Projectile::Projectile(const Asset::Texture& texture, glm::dvec2 pos, glm::dvec2 speed) : SolidObject(glm::dvec2(0), pos, 0, speed) {
	SetOwner(GameState::player->GetId());
	isdead = false;
	m_sprite.SetTexture(texture);
	m_size = texture.size;
	timer = 3;
	m_type = object_type::projectile;
	m_last_position = glm::vec2(pos);
}

void Projectile::Destroy() {
	isdead = true;
}

void Projectile::Draw() {
	m_sprite.DrawSprite(m_size, m_position, m_rotation);
}

void Projectile::Update() {
	timer -= GameState::deltaTime;
	
	float rayLen = glm::length(this->GetSpeed()) * 0.05;
	glm::vec2 projectileDirection = glm::normalize(glm::dvec2(m_acceleration));
	
	std::vector<glm::vec2> rayLine;
	rayLine.push_back(glm::vec2(m_position.x + m_size.x/2.0, m_position.y + m_size.y/2.0));
	rayLine.push_back(glm::vec2(m_position.x + m_size.x/2.0 + projectileDirection.x * rayLen, m_position.y + m_size.y/2.0 + projectileDirection.y * rayLen));
	
	UpdateProjectileRay(rayLine);
	
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
