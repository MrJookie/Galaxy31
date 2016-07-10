#include "Ship.hpp"

Ship::Ship() {
	init();
}

Ship::Ship(std::string sprite_file) : m_sprite(sprite_file) {
	init();
}

void Ship::init() {
	m_speed = glm::vec2(0);
	
	if(!initialized_statics) {
		initialized_statics = true;
		m_engine_propulsion = Sprite("Assets/propulsion.png");
	}
}

bool Ship::initialized_statics = false;
Sprite Ship::m_engine_propulsion;

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
	m_last_acceleration = glm::length(acceleration);
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
	m_sprite.SetPosition(posX - m_sprite.GetSize().x * 0.5, posY - m_sprite.GetSize().y * 0.5);
}
glm::vec2 Ship::GetPosition() {
	const glm::vec2 &pos = m_sprite.GetPosition();
	const glm::vec2 &size = m_sprite.GetSize();
	return glm::vec2( pos.x + 0.5 * size.x, pos.y + 0.5 * size.y );
}


void Ship::Draw(Camera &cam) {
	m_sprite.DrawSprite(m_sprite_shader, cam.GetView(), cam.GetProjection());
	const float a = 0.8;
	
	if(m_last_acceleration > 0) {
		m_engine_propulsion.SetSize(GetSize().x, GetSize().y * 0.7 * std::min(1.0f, m_last_acceleration*a) );
		float theta = (m_sprite.GetRotation() + 90) * 3.141592 / 180.0;
		const glm::vec2 &pos = m_sprite.GetPosition();
		const glm::vec2 &size = m_sprite.GetSize();
		const glm::vec2 &psize = m_engine_propulsion.GetSize();
		const float A = size.y * 0.5 + psize.y*0.5;
		m_engine_propulsion.SetPosition( 
										pos.x + A*cos(theta), 
										pos.y + size.y * 0.5 + A*sin(theta) - psize.y*0.5
										);
		m_engine_propulsion.SetRotation( m_sprite.GetRotation() );
		m_engine_propulsion.DrawSprite(m_sprite_shader, cam.GetView(), cam.GetProjection());
	}
}

void Ship::SetSpriteShader(unsigned int spriteShader) {
	m_sprite_shader = spriteShader;
}

void Ship::Process(double dt) {
	SetPosition(GetPosition().x + dt * m_speed.x, GetPosition().y + dt * m_speed.y);
}
