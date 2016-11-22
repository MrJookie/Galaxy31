#ifndef ASTEROID_HPP
#define ASTEROID_HPP

#include "Asset.hpp"
#include "Sprite.hpp"
#include "SolidObject.hpp"

class Asteroid : public SolidObject {
	public:
		Asteroid() {}
		Asteroid(const Asteroid& projectile);
		Asteroid(const Asset::Texture& texture, glm::dvec2 pos = glm::dvec2(0), float rotation = 0.0f, float rotationSpeed = 0.0f, glm::dvec2 speed = glm::dvec2(0));
		void Draw();
		void Update();
		void Destroy();
		Sprite* GetSprite();
		
	private:
		bool isdead;
		Sprite m_sprite;
};

#endif
