#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>
#include <vector>

class Object {
	public:
		Object(glm::vec2 size = {0,0}, glm::vec2 position = {0,0}, double rotation = 0, glm::vec2 speed = {0,0}) : 
			m_size(size), m_position(position), m_rotation(rotation), m_speed(speed), m_rotation_speed(0) {}
		~Object() {}

		void SetSize(glm::dvec2 size);
		void SetPosition(glm::dvec2 position);
		void SetRotation(double rotation);
		void SetSpeed(glm::dvec2 speed);
		void SetAcceleration(glm::dvec2 acceleration);
		void CopyObjectState(Object &obj);
		void InterpolateToState(Object &obj, double interpolation);
		
		glm::dvec2 GetSize() const;
		glm::dvec2 GetPosition() const;
		double GetRotation() const;
		glm::dvec2 GetSpeed() const;
		glm::dvec2 GetAcceleration() const;
		void SetRotationSpeed( double rotation_speed );
		
		void Accelerate(glm::dvec2 acceleration);

		void Draw() {};
		
		void Process(double dt = 0);
		
		unsigned int GetId() { return id; }
		void SetId(unsigned int id) { this->id = id; }
		
		unsigned int GetOwner() { return owner; }
		void SetOwner(unsigned int owner) { this->owner = owner; }
		
		unsigned int GetTicks() { return m_ticks; }
		unsigned int SetTicks(unsigned int tick) { m_ticks = tick; }
		unsigned int AddTicks(unsigned int tick) { m_ticks += tick; }
		void UpdateTicks();

	protected:
		uint32_t id;
		uint32_t owner;
		
		glm::dvec2 m_size;
		glm::dvec2 m_position;
		double m_rotation;
		double m_rotation_speed;
		glm::dvec2 m_speed;
		uint32_t m_ticks;
		
		glm::dvec2 m_acceleration;
		
		glm::dvec2 local_to_world_coord(const glm::dvec2& local_coord);
		
	private:

};

#endif
