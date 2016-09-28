#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>
#include <vector>

enum class object_type {
	unknown,
	projectile,
	ship,
	solidobject
};

class Object {
	public:
		Object(glm::dvec2 size = {0,0}, glm::dvec2 position = {0,0}, double rotation = 0, glm::dvec2 speed = {0,0}) : 
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
		
		unsigned int GetId() { return m_id; }
		void SetId(unsigned int id) { this->m_id = id; }
		
		unsigned int GetOwner() { return m_owner; }
		void SetOwner(unsigned int owner) { this->m_owner = owner; }
		
		object_type GetType() { return m_type; }
		void SetType(object_type type) { m_type = type; }
		
		uint32_t GetTicks() { return m_ticks; }
		uint32_t SetTicks(uint32_t tick) { m_ticks = tick; }
		uint32_t AddTicks(uint32_t tick) { m_ticks += tick; }
		void UpdateTicks();

	protected:
		object_type m_type;
		uint32_t m_id;
		uint32_t m_owner;
		
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
