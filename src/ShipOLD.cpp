#include "Ship.hpp"

Ship::Ship() {
	m_speed = glm::vec2(0);

	//m_engine_propulsion = Sprite();
	//m_engine_propulsion.SetTexture(GameState::asset.GetTexture("Assets/Textures/propulsion.png"));
}

//Sprite Ship::m_engine_propulsion;

Ship::~Ship() {}

void Ship::SetSpriteTexture(GLuint textureID) {
	m_sprite.SetTexture(textureID);
}

void Ship::SetSpriteShader(GLuint spriteShader) {
    m_sprite_shader = spriteShader;
}

void Ship::SetSize(glm::vec2 size) {
    m_sprite.SetSize(size);
}

void Ship::SetPosition(glm::vec2 position) {
    m_sprite.SetPosition(glm::vec2(position.x - m_sprite.GetSize().x * 0.5, position.y - m_sprite.GetSize().y * 0.5));
}

void Ship::SetRotation(float rotation) {
    m_sprite.SetRotation(rotation);
}

void Ship::SetSpeed(glm::vec2 speed) {
    m_speed = speed;
}

void Ship::SetAcceleration(float acceleration) {
    m_acceleration = acceleration;
}

glm::vec2 Ship::GetSize() const {
    return m_sprite.GetSize();
}

glm::vec2 Ship::GetPosition() const {
    const glm::vec2 &pos = m_sprite.GetPosition();
    const glm::vec2 &size = m_sprite.GetSize();

    return glm::vec2( pos.x + 0.5 * size.x, pos.y + 0.5 * size.y );
}

float Ship::GetRotation() const {
    return m_sprite.GetRotation();
}

glm::vec2 Ship::GetSpeed() const {
    return m_speed;
}

float Ship::GetAcceleration() const {
    return m_acceleration;
}

void Ship::Accelerate(glm::vec2 acceleration) {
    m_speed += acceleration;
    m_last_acceleration = glm::length(acceleration);
}

void Ship::Draw() {
    m_sprite.DrawSprite(m_sprite_shader, GameState::camera.GetViewMatrix(), GameState::camera.GetProjection());
/*
	const float a = 0.4;
    if(m_last_acceleration > 0.1) {
        m_engine_propulsion.SetSize(GetSize().x*0.5, GetSize().y * 0.6 * std::max(0.01f, m_last_acceleration*a) );
        float theta = (m_sprite.GetRotation() + 90) * 3.141592 / 180.0;
        const glm::vec2 &pos = m_sprite.GetPosition();
        const glm::vec2 &size = m_sprite.GetSize();
        const glm::vec2 &psize = m_engine_propulsion.GetSize();
        const float A = size.y * 0.5 + psize.y*0.5;
        m_engine_propulsion.SetPosition(
            pos.x + A*cos(theta) + psize.x*0.5,
            pos.y + size.y * 0.5 + A*sin(theta) - psize.y*0.5
        );
        m_engine_propulsion.SetRotation( m_sprite.GetRotation() );
        m_engine_propulsion.DrawSprite(m_sprite_shader, GameState::camera.GetViewMatrix(), GameState::camera.GetProjection());
    }
*/
}



void Ship::Process() {
    SetPosition(glm::vec2(this->GetPosition().x + GameState::deltaTime * this->GetSpeed().x, this->GetPosition().y + GameState::deltaTime * this->GetSpeed().y));
}
