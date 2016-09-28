#include "Projectile.hpp"
#include <iostream>
#include "GameState.hpp"

Projectile::Projectile(const Asset::Texture& texture, glm::dvec2 pos, glm::dvec2 speed) : SolidObject(glm::dvec2(0), pos, 0, speed) {
	isdead = false;
	m_sprite.SetTexture(texture);
	m_size = texture.size;
	timer = 20;
	m_type = object_type::projectile;
}

void Projectile::Destroy() {
	isdead = true;
}

void Projectile::Draw() {
	m_sprite.DrawSprite(m_size, m_position, m_rotation);
	timer -= GameState::deltaTime;
}

Projectile::Projectile(const Projectile& c) {
	*this = c;
}
bool Projectile::IsDead() {
	return isdead || timer < 0;
}
