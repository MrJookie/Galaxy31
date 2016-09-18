#include "Projectile.hpp"
#include <iostream>
#include "GameState.hpp"

Projectile::Projectile(const Asset::Texture& texture, glm::vec2 pos, glm::vec2 speed) : SolidObject(glm::vec2(0), pos, 0, speed) {
	
	m_sprite.SetTexture(texture);
	m_size = texture.size;
	timer = 20;
}

void Projectile::Draw() {
	m_sprite.DrawSprite(m_size, m_position, m_rotation);
	timer -= GameState::deltaTime;
}

Projectile::Projectile(const Projectile& c) {
	*this = c;
}
bool Projectile::IsDead() {
	return timer < 0;
}
