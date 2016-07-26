#include "Object.hpp"
#include "GameState.hpp"

Object::Object(glm::vec2 size, glm::vec2 position, float rotation, glm::vec2 speed, float acceleration) : m_size(size), m_position(position), m_rotation(rotation), m_speed(speed), m_acceleration(acceleration) {
	
}

Object::~Object() {}

void Object::SetSize(glm::vec2 size) {
    m_size.x = size.x;
    m_size.y = size.y;
}

void Object::SetPosition(glm::vec2 position) {
    m_position.x = position.x - this->GetSize().x * 0.5;
    m_position.y = position.y - this->GetSize().y * 0.5;
}

void Object::SetRotation(float rotation) {
    m_rotation = rotation;
}

void Object::SetSpeed(glm::vec2 speed) {
    m_speed = speed;
}

void Object::SetAcceleration(float acceleration) {
    m_acceleration = acceleration;
}

glm::vec2 Object::GetSize() const {
    return m_size;
}

glm::vec2 Object::GetPosition() const {
    return glm::vec2( m_position.x + 0.5 * m_size.x, m_position.y + 0.5 * m_size.y );
}

float Object::GetRotation() const {
    return m_rotation;
}

glm::vec2 Object::GetSpeed() const {
    return m_speed;
}

float Object::GetAcceleration() const {
    return m_acceleration;
}

void Object::Accelerate(glm::vec2 acceleration) {
    m_speed += acceleration;
    m_last_acceleration = glm::length(acceleration);
}

void Object::Process() {
	double dt = GameState::deltaTime;
	m_position.x = m_position.x + dt * m_speed.x;
	m_position.y = m_position.y + dt * m_speed.y;
	m_rotation += m_rotation_speed * dt;
}

void Object::SetRotationSpeed( float rotation_speed ) {
	m_rotation_speed = rotation_speed;
}

glm::vec2 Object::local_to_world_coord(const glm::vec2& local_coord) {
	glm::vec2 global_coord;
	float angle = m_rotation * 3.141592f / 180.0f;
	float s = sin(angle), c = cos(angle);
	// A*cos(x + r) = (A*cos(x))cos(r) - (A*sin(x))sin(r)
	// A*sin(x + r) = (A*sin(x))cos(r) + (A*cos(x))sin(r)
	
	global_coord.x = (local_coord.x * c - local_coord.y * s) + m_position.x + m_size.x*0.5;
	global_coord.y = (local_coord.y * c + local_coord.x * s) + m_position.y + m_size.y*0.5;
	return global_coord;
}
