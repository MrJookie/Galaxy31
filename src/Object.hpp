#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>
#include <vector>

class Object {
	public:
		Object(glm::vec2 size = {0,0}, glm::vec2 position = {0,0}, float rotation = 0, glm::vec2 speed = {0,0}) : 
			m_size(size), m_position(position), m_rotation(rotation), m_speed(speed), m_rotation_speed(0) {}
		~Object() {}

		void SetSize(glm::vec2 size);
		void SetPosition(glm::vec2 position);
		void SetRotation(float rotation);
		void SetSpeed(glm::vec2 speed);
		void SetAcceleration(glm::vec2 acceleration);
		void CopyObjectState(Object &obj);
		void InterpolateToState(Object &obj, float interpolation);
		
		glm::vec2 GetSize() const;
		glm::vec2 GetPosition() const;
		float GetRotation() const;
		glm::vec2 GetSpeed() const;
		glm::vec2 GetAcceleration() const;
		void SetRotationSpeed( float rotation_speed );
		
		void Accelerate(glm::vec2 acceleration);

		void Draw() {};
		
		void Process(float dt = 0);
		unsigned int GetId() { return id; }
		void SetId(unsigned int id) { this->id = id; }
		unsigned int GetTicks() { return m_ticks; }
		unsigned int SetTicks(unsigned int tick) { m_ticks = tick; }
		unsigned int AddTicks(unsigned int tick) { m_ticks += tick; }
		void UpdateTicks();

	protected:
		uint32_t id;
		
		glm::vec2 m_size;
		glm::vec2 m_position;
		float m_rotation;
		float m_rotation_speed;
		glm::vec2 m_speed;
		uint32_t m_ticks;
		
		glm::vec2 m_acceleration;
		
		glm::vec2 local_to_world_coord(const glm::vec2& local_coord);
		
	private:

};

#endif
