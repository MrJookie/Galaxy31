#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "Asset.hpp"
#include "Sprite.hpp"
#include "SolidObject.hpp"

class Projectile : public SolidObject {
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
