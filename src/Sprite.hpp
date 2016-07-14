#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "GameState.hpp"

class Sprite {
	public:
		Sprite();
		~Sprite();
		Sprite& operator=(Sprite &&);
		Sprite& operator=(const Sprite& o) = default;
		
		void DrawSprite(glm::vec2 size, glm::vec2 position, float rotation);
		
		void SetTexture(GLuint textureID);
		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		
		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;

	private:
		GLuint m_vao, m_vbo, m_ebo;
		GLuint m_texture;

		glm::vec2 m_size;
		glm::vec2 m_position;
		float m_rotation;
};

#endif
