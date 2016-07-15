#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <glm/glm.hpp>
#include <list>
#include "Asset.hpp"

class Sprite {
	public:
		Sprite();
		~Sprite();
		Sprite& operator=(Sprite &&);
		Sprite& operator=(Sprite const& o) = default;
		
		void DrawSprite();
		void DrawSprite(glm::vec2 size, glm::vec2 position, float rotation);
		
		void SetTexture(const Asset::Texture &tex);
		void AddTexture(GLuint tex);
		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		
		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;

	private:
		static bool first_time;
		static GLuint m_vao, m_vbo, m_ebo;
		std::list<GLuint> m_textures;

		glm::vec2 m_size;
		glm::vec2 m_position;
		float m_rotation;
};

#endif
