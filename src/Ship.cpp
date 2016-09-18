#include "Ship.hpp"
#include "GameState.hpp"
#include <glm/gtx/vector_angle.hpp>

Ship::Ship(glm::vec2 position, float rotation, const Chassis& chassis) {
	m_position = position;
	m_rotation = rotation;
	m_speed = glm::vec2(0);
	m_acceleration = {0,0};
	m_chassis = chassis;
	m_chassis.sprite = std::move(chassis.sprite);
	m_chassis.sprite.SetPosition(m_position);
	m_chassis.sprite.SetRotation(m_rotation);
	m_chassis.sprite.AddTexture(m_chassis.texture);
	m_size = m_chassis.sprite.GetSize();
	m_stabilizers_on = false;
	m_rotation_speed_coefficient = 180;
	m_max_distance_acceleration = 768;
	m_downshift_coefficient = 0.8f;
	m_max_speed_coefficient = 6000.0f;
	m_acceleration_speed_coefficient = 3.0f;
	m_brake_coefficient = 4.0f;
	m_engine_propulsion_coefficient = 6.2f;
	m_engine_propulsion.SetTexture(GameState::asset.GetTexture("propulsion.png"));
}

Ship::~Ship() {}

void Ship::Draw() {
	m_chassis.sprite.DrawSprite(m_size, m_position, m_rotation); //chassi only
	//draw loaded mountables within propulsion on engine mountable
	float acceleration = glm::length(this->GetAcceleration()) * 0.5f;
	if(acceleration > 0.01) {
        m_engine_propulsion.SetSize(glm::vec2(this->GetSize().x * 0.5, this->GetSize().y * 0.5 * std::max(0.01f,  acceleration * 0.0003f * m_engine_propulsion_coefficient)) );
        float theta = (this->GetRotation() + 90) * 3.141592 / 180.0;
        const glm::vec2 &psize = m_engine_propulsion.GetSize();
        const float A = this->GetSize().y * 0.5 + psize.y*0.5;
        m_engine_propulsion.SetPosition(glm::vec2(
            m_position.x + A*cos(theta) + psize.x*0.5,
            m_position.y + this->GetSize().y * 0.5 + A*sin(theta) - psize.y*0.5
        ));
        m_engine_propulsion.SetRotation( this->GetRotation() );
        m_engine_propulsion.DrawSprite(m_engine_propulsion.GetSize(), m_engine_propulsion.GetPosition(), this->GetRotation());
    }
}

void Ship::Process() {
	glm::vec2 pos_to_mouse_vector = glm::vec2( GameState::worldMousePosition.x - this->GetPosition().x, GameState::worldMousePosition.y - this->GetPosition().y );
	glm::vec2 direction = glm::normalize(pos_to_mouse_vector);
	float distance = glm::length(pos_to_mouse_vector);
	float angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), direction ) );

	float angle_to = (angle+90.0f) - this->GetRotation();
	while(angle_to > 180) angle_to -= 360;
	while(angle_to < -180) angle_to += 360;

	float angle_speed = 0;
	const float angle_thresshold = 2.0f;
	if(angle_to > angle_thresshold) angle_speed = m_rotation_speed_coefficient;
	else if(angle_to < -angle_thresshold) angle_speed = -m_rotation_speed_coefficient;
	this->SetRotationSpeed( angle_speed );
	
	const Uint8* state = SDL_GetKeyboardState(NULL);
	
	if(distance > m_max_distance_acceleration) {
		distance = m_max_distance_acceleration;
	}
	
	// this->Accelerate( -this->GetSpeed() * m_downshift_coefficient * float(GameState::deltaTime) );
		
	if(state[SDL_SCANCODE_W]) {
		
		// this->Accelerate(direction * distance * m_acceleration_speed_coefficient * float(GameState::deltaTime));
		this->SetAcceleration(direction * distance * m_acceleration_speed_coefficient);
	} else {
		// dampening
		this->SetAcceleration(this->GetAcceleration() *  0.95f);
	}
	
	if(state[SDL_SCANCODE_S]) {
		this->SetAcceleration({0,0});
		this->Accelerate( -this->GetSpeed() * m_brake_coefficient * float(GameState::deltaTime) );
	}

    Object::Process();
    
    if(glm::length(this->GetSpeed()) > m_max_speed_coefficient) {
		this->SetSpeed(glm::normalize(this->GetSpeed()) * m_max_speed_coefficient);
	}
}

/*
void Ship::ProcessOLD() {
	glm::vec2 pos_to_mouse_vector = glm::vec2( GameState::worldMousePosition.x - this->GetPosition().x, GameState::worldMousePosition.y - this->GetPosition().y );
	glm::vec2 direction = glm::normalize(pos_to_mouse_vector);
	float distance = glm::length(pos_to_mouse_vector);
	float angle;
	
	if(m_stabilizers_on) {
		angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), glm::normalize(-m_speed) ) );
	} else {
		angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), direction ) );
	}
	
	float angle_to = (angle+90.0f) - this->GetRotation();
	while(angle_to > 180) angle_to -= 360;
	while(angle_to < -180) angle_to += 360;

	float angle_speed = 0;
	const float angle_thresshold = 1.0f;
	if(angle_to > angle_thresshold) angle_speed = m_rotation_speed_coefficient;
	else if(angle_to < -angle_thresshold) angle_speed = -m_rotation_speed_coefficient;
	this->SetRotationSpeed( angle_speed );
	
	if(m_stabilizers_on) {
		this->Accelerate( -m_speed * 0.2f * float(GameState::deltaTime) );
	} else {
		if(distance > (this->GetSize().y * 0.5)) {
			this->Accelerate(direction * distance * m_acceleration_speed_coefficient*0.001f);
		}
	}
	
    Object::Process();
}
*/

void Ship::Fire() {
	glm::vec2 world_coord = local_to_world_coord(glm::vec2(0, -m_size.y*0.5));
	Projectile projectile(GameState::asset.GetTexture("projectile.png"), world_coord, glm::normalize(world_coord - GetPosition())*1000.0f + m_speed);
	projectile.SetAcceleration(m_acceleration);
	projectile.SetRotation(GetRotation());
	GameState::projectiles.push_back(projectile);
}

void Ship::Stabilizers() {
	m_stabilizers_on = !m_stabilizers_on;
}

Ship::Chassis::Chassis() {}
Ship::Chassis::Chassis(std::string _name, std::string _texture, std::string _skin) {
	name = _name;
	Asset::Texture tex = GameState::asset.GetTexture(_texture);
	texture = tex.id;
	skin = GameState::asset.GetTexture(_skin).id;
	sprite.SetSize(tex.size);
}
