#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "Object.hpp"
#include "Asset.hpp"
#include "Sprite.hpp"

class Projectile : public Object {
	public:
		Projectile() {}
		Projectile(const Projectile& projectile);
		Projectile(const Asset::Texture& texture, glm::vec2 pos, glm::vec2 speed);
		void Draw();
		bool IsDead();
		
	private:
		Sprite m_sprite;
		double timer;
};

#endif
