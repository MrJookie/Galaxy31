#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "GameState.hpp"
#include "Sprite.hpp"

class Object {
	public:
		Object();
		~Object();

		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		void SetSpeed(glm::vec2 speed);
		void SetAcceleration(float acceleration);
		
		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;
		glm::vec2 GetSpeed() const;
		float GetAcceleration() const;
		
		void Accelerate(glm::vec2 acceleration);

	protected:
		glm::vec2 m_size;
		glm::vec2 m_position;
		float m_rotation;
		glm::vec2 m_speed;
		
		float m_acceleration;
		float m_last_acceleration;
		
	private:

};

#endif
