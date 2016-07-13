#include "Object.hpp"

Object::Object() {}

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
