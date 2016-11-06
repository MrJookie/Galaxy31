#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "Asset.hpp"
#include "Sprite.hpp"
#include "SolidObject.hpp"

class Projectile : public SolidObject {
	public:
		Projectile() {}
		Projectile(const Projectile& projectile);
		Projectile(const Asset::Texture& texture, glm::dvec2 pos = glm::dvec2(0), glm::dvec2 speed = glm::dvec2(0));
		void Draw();
		void Update();
		bool IsDead();
		void Destroy();
		Sprite* GetSprite();
		
	private:
		bool isdead;
		Sprite m_sprite;
		double timer;
		
		glm::vec2 m_last_position;
};

#endif
