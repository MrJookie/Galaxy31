#include "Ship.hpp"
#include <glm/gtx/vector_angle.hpp>

Ship::Ship(glm::vec2 position, float rotation, const Chassis& chassis) {
	m_position = position;
	m_rotation = rotation;
	m_speed = glm::vec2(0);
	m_acceleration = 0.001;
	m_last_acceleration = 0;
	m_chassis = chassis;
	m_chassis.sprite = std::move(chassis.sprite);
	m_chassis.sprite.SetPosition(m_position);
	m_chassis.sprite.SetRotation(m_rotation);
	m_chassis.sprite.AddTexture(m_chassis.texture);
	m_size = m_chassis.sprite.GetSize();
	m_stabilizers_on = false;
	m_rotation_speed_coefficient = 180;
	m_engine_propulsion.SetTexture(GameState::asset.GetTexture("propulsion.png"));
}

Ship::~Ship() {}

void Ship::Draw() {
	m_chassis.sprite.DrawSprite(m_size, m_position, m_rotation); //chassi only
	//draw loaded mountables within propulsion on engine mountable
	
	const float a = 0.1;
	if(m_last_acceleration > 0.1) {
        m_engine_propulsion.SetSize(glm::vec2(this->GetSize().x*0.5, this->GetSize().y * 0.5 * std::max(0.01f, m_last_acceleration*a)) );
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
			this->Accelerate(direction * distance * this->GetAcceleration());
		}
	}
	
    Object::Process();
}

void Ship::Process2() {
	glm::vec2 pos_to_mouse_vector = glm::vec2( GameState::worldMousePosition.x - this->GetPosition().x, GameState::worldMousePosition.y - this->GetPosition().y );
	glm::vec2 direction = glm::normalize(pos_to_mouse_vector);
	float distance = glm::length(pos_to_mouse_vector);
	float angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), direction ) );

	float angle_to = (angle+90.0f) - this->GetRotation();
	while(angle_to > 180) angle_to -= 360;
	while(angle_to < -180) angle_to += 360;

	float angle_speed = 0;
	const float angle_thresshold = 1.0f;
	if(angle_to > angle_thresshold) angle_speed = m_rotation_speed_coefficient;
	else if(angle_to < -angle_thresshold) angle_speed = -m_rotation_speed_coefficient;
	this->SetRotationSpeed( angle_speed );

	const int MAX_DISTANCE = 768; //min. screen length (width) allowed by our game, App::m_initialWindowSize.y
	const float DOWNSHIFT = 0.8f;
	const float MAX_SPEED = 4000.0f; //todo
	const float ACCELERATION = 5000.0f;
	const float BRAKE = 4.0f; 
	
	const Uint8* state = SDL_GetKeyboardState(NULL);
	
	if(distance > MAX_DISTANCE) {
		distance = MAX_DISTANCE;
	}
	
	this->Accelerate( -this->GetSpeed() * DOWNSHIFT * float(GameState::deltaTime) );

	if(state[SDL_SCANCODE_W]) {
		this->Accelerate(direction * distance * this->GetAcceleration() * ACCELERATION * float(GameState::deltaTime));
	}
	
	if(state[SDL_SCANCODE_S]) {
		this->Accelerate(glm::vec2(0,0));
		this->Accelerate( -this->GetSpeed() * BRAKE * float(GameState::deltaTime) );
	}

    Object::Process();
}

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
