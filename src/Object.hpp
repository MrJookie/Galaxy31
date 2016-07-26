#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>

class Object {
	public:
		Object(glm::vec2 size = glm::vec2(0), glm::vec2 position = glm::vec2(0), float rotation = 0, glm::vec2 speed = glm::vec2(0), float acceleration = 0) {}
		~Object() {}

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
		void SetRotationSpeed( float rotation_speed );
		
		void Accelerate(glm::vec2 acceleration);
		
		void Process();

	protected:
		glm::vec2 m_size;
		glm::vec2 m_position;
		float m_rotation;
		float m_rotation_speed;
		glm::vec2 m_speed;
		
		float m_acceleration;
		float m_last_acceleration;
		
		glm::vec2 local_to_world_coord(const glm::vec2& local_coord);
		
	private:

};

#endif
