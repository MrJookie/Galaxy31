#include "Object.hpp"

#ifndef SERVER
#include "GameState.hpp"
#endif

#include <chrono>

void Object::SetSize(glm::dvec2 size) {
    m_size.x = size.x;
    m_size.y = size.y;
}

void Object::SetPosition(glm::dvec2 position) {
    m_position.x = position.x - this->GetSize().x * 0.5;
    m_position.y = position.y - this->GetSize().y * 0.5;
}

void Object::SetRotation(double rotation) {
    m_rotation = rotation;
}

void Object::SetSpeed(glm::dvec2 speed) {
    m_speed = speed;
}

void Object::SetAcceleration(glm::dvec2 acceleration) {
    m_acceleration = acceleration;
}

glm::dvec2 Object::GetSize() const {
    return m_size;
}

glm::dvec2 Object::GetPosition() const {
    return glm::dvec2( m_position.x + 0.5 * m_size.x, m_position.y + 0.5 * m_size.y );
}

double Object::GetRotation() const {
    return m_rotation;
}

glm::dvec2 Object::GetSpeed() const {
    return m_speed;
}

glm::dvec2 Object::GetAcceleration() const {
    return m_acceleration;
}

void Object::Accelerate(glm::dvec2 acceleration) {
    m_speed += acceleration;
}

void Object::Process(double dt) {
	#ifndef SERVER
	if(dt == 0)
		dt = GameState::deltaTime;
	#endif
	m_speed += m_acceleration * dt;
	m_position += m_speed * dt;
	m_rotation += m_rotation_speed * dt;
}

void Object::SetRotationSpeed( double rotation_speed ) {
	m_rotation_speed = rotation_speed;
}

void Object::InterpolateToState(Object &obj, double interpolation) {
	m_speed += (obj.m_speed - m_speed) * interpolation;
	m_position += (obj.m_position - m_position) * interpolation;
	m_rotation += (obj.m_rotation - m_rotation) * interpolation;
	m_rotation_speed = obj.m_rotation_speed;
	m_acceleration += (obj.m_acceleration - m_acceleration) * interpolation;
}

void Object::CopyObjectState(Object &obj) {
	*this = obj;
}

static std::chrono::high_resolution_clock::time_point first_tick = std::chrono::high_resolution_clock::now();
void Object::UpdateTicks() {
	m_ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - first_tick).count();
}

glm::dvec2 Object::local_to_world_coord(const glm::dvec2& local_coord) {
	glm::dvec2 global_coord;
	double angle = m_rotation * 3.141592 / 180.0;
	double s = sin(angle), c = cos(angle);
	// A*cos(x + r) = (A*cos(x))cos(r) - (A*sin(x))sin(r)
	// A*sin(x + r) = (A*sin(x))cos(r) + (A*cos(x))sin(r)
	
	/*
	 cos  -sin
	 sin  cos
	*/
	
	global_coord.x = (local_coord.x * c - local_coord.y * s) + m_position.x + m_size.x*0.5f;
	global_coord.y = (local_coord.y * c + local_coord.x * s) + m_position.y + m_size.y*0.5f;
	return global_coord;
}


