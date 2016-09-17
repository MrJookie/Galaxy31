#include "Object.hpp"

#ifndef SERVER
#include "GameState.hpp"
#endif

#include <chrono>

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

void Object::SetAcceleration(glm::vec2 acceleration) {
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

glm::vec2 Object::GetAcceleration() const {
    return m_acceleration;
}

void Object::Accelerate(glm::vec2 acceleration) {
    m_speed += acceleration;
}

void Object::Process(float dt) {
	#ifndef SERVER
	if(dt == 0)
		dt = GameState::deltaTime;
	#endif
	m_speed += m_acceleration * dt;
	m_position += m_speed * dt;
	m_rotation += m_rotation_speed * dt;
}

void Object::SetRotationSpeed( float rotation_speed ) {
	m_rotation_speed = rotation_speed;
}

void Object::InterpolateToState(Object &obj, float interpolation) {
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

bool Object::DoObjectsIntersect(Object* obj) {
	return !(m_position.x > obj->m_position.x + obj->m_size.x
        || m_position.x+m_size.x < obj->m_position.x
        || m_position.y > obj->m_position.y + obj->m_size.y
        || m_position.y + m_size.y < obj->m_position.y);
}

bool Object::DoLinesIntersect(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4) {
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
	 
	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	if(d == 0) {
		return false;
	}
	 
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;
	 
	if(x < std::min(x1, x2) || x > std::max(x1, x2) || x < std::min(x3, x4) || x > std::max(x3, x4)) {
		return false;
	}
	
	if(y < std::min(y1, y2) || y > std::max(y1, y2) || y < std::min(y3, y4) || y > std::max(y3, y4)) {
		return false;
	}

	// point of intersection
	//return glm::vec2(x, y);

	return true;
}
