#include "Ship.hpp"

Ship::Ship() {
	m_speed = glm::vec2(0);
}
Ship::Ship(std::string sprite_file) : m_sprite(sprite_file) {
	m_speed = glm::vec2(0);
}
Ship::~Ship() {
	
}
		
void Ship::SetSpeed(glm::vec2 speed) {
	m_speed = speed;
}

glm::vec2 Ship::GetSpeed() {
	return m_speed;
}

void Ship::Accelerate(glm::vec2 acceleration) {
	m_speed += acceleration;
}


void Ship::SetRotation(float rotation) {
	m_sprite.SetRotation(rotation);
}
float Ship::GetRotation() {
	return m_sprite.GetRotation();
}


void Ship::SetSize(int sizeX, int sizeY) {
	m_sprite.SetSize(sizeX, sizeY);
}

glm::vec2 Ship::GetSize() {
	return m_sprite.GetSize();
}

void Ship::SetPosition(float posX, float posY) {
	m_sprite.SetPosition(posX, posY);
}
glm::vec2 Ship::GetPosition() {
	//~ const glm::vec2 &pos = m_sprite.GetPosition();
	//~ const glm::vec2 &size = m_sprite.GetSize();
	//~ return glm::vec2( pos.x + 0.5 * size.x, pos.y + 0.5 + size.y );
	return m_sprite.GetPosition();
}


void Ship::Draw(Camera &cam) {
	m_sprite.DrawSprite(m_sprite_shader, cam.GetView(), cam.GetProjection());
}

void Ship::SetSpriteShader(unsigned int spriteShader) {
	m_sprite_shader = spriteShader;
}

void Ship::DrawSprite(GLuint shader, glm::mat4 view, glm::mat4 projection) {
	m_sprite.DrawSprite(shader, view, projection);
}
