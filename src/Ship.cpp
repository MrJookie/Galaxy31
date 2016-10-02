#include "Ship.hpp"
#include "GameState.hpp"
#include <glm/gtx/vector_angle.hpp>
#include <random>
#include "Network.hpp"
#include "EventSystem/Event.hpp"
#include "commands/commands.hpp"
Ship::Ship(glm::dvec2 position, double rotation, const Chassis& chassis) {
	m_type = object_type::ship;
	m_position = position;
	m_rotation = rotation;
	m_speed = glm::dvec2(0);
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
	m_downshift_coefficient = 0.8;
	m_max_speed_coefficient = 6000.0;
	m_acceleration_speed_coefficient = 1000.0;
	m_brake_coefficient = 4.0;
	m_engine_propulsion_coefficient = 6.2;
	m_engine_propulsion.SetTexture(GameState::asset.GetTexture("propulsion2.png"));
}

std::default_random_engine re;

Ship::~Ship() {}
void Ship::Draw() {
	std::uniform_real_distribution<double> unif(0.8,1.0);
   
	m_chassis.sprite.DrawSprite(m_size, m_position, m_rotation); //chassi only
	//draw loaded mountables within propulsion on engine mountable
	double acceleration = glm::length(this->GetAcceleration()) * 0.5;
	if(acceleration > 0.01) {
		double y = m_engine_propulsion_coefficient * unif(re) * std::max(0.01,  acceleration * 0.03);
		// m_engine_propulsion.SetSize(glm::dvec2(this->GetSize().x * 0.1, this->GetSize().y * 0.5 * std::max(0.01,  acceleration * 0.0003 * m_engine_propulsion_coefficient)) );
		m_engine_propulsion.SetSize(glm::dvec2(this->GetSize().x * 0.2, y) );
		// double theta = glm::radians(this->GetRotation()+90.0);
		const glm::vec2 &psize = m_engine_propulsion.GetSize();
		// const double A = this->GetSize().y * 0.5 + psize.y*0.5;
		
		// m_engine_propulsion.SetPosition(glm::dvec2(
			// m_position.x + A*cos(theta) + psize.x*0.5,
			// m_position.y + this->GetSize().y * 0.5 + A*sin(theta) - psize.y*0.5
		// ));
		
		glm::dvec2 world = local_to_world_coord( glm::dvec2((this->GetSize().x-psize.x)/2.0+(psize.x-this->GetSize().x)*0.5, (this->GetSize().y+y)*0.5) );
		// glm::dvec2 world = glm::dvec2((this->GetSize().x-psize.x)/2.0-this->GetSize().x*0.5, this->GetSize().y*0.5) + m_position;
		
		world.x -= psize.x*0.5;
		world.y -= y*0.5;
		m_engine_propulsion.SetPosition( world );
		
		// m_engine_propulsion.SetRotation( this->GetRotation() );
		m_engine_propulsion.DrawSprite(m_engine_propulsion.GetSize(), m_engine_propulsion.GetPosition(), this->GetRotation());
	} else {
		m_engine_propulsion.RemoveFromDrawing();
	}
}

void Ship::Process() {
	glm::dvec2 pos_to_mouse_vector = glm::dvec2( GameState::worldMousePosition.x - this->GetPosition().x, GameState::worldMousePosition.y - this->GetPosition().y );
	glm::dvec2 direction = glm::normalize(pos_to_mouse_vector);
	double distance = glm::length(pos_to_mouse_vector);
	
	/*
		+--> x
		|
		v y
	 */
	 
	double angle = glm::degrees( glm::orientedAngle( glm::dvec2(0.0f, -1.0f), direction ) );
	// cout << this->GetRotation() << endl;
	double angle_to = (angle) - this->GetRotation();
	while(angle_to > 180.0) angle_to -= 360.0;
	while(angle_to < -180.0) angle_to += 360.0;

	double angle_speed = 0;
	
	m_rotation_speed_coefficient = Command::Get("rotation_speed").d;
	m_acceleration_speed_coefficient = Command::Get("speed").d;
	
	const double angle_thresshold = m_rotation_speed_coefficient * GameState::deltaTime;
	if(angle_to > angle_thresshold) angle_speed = m_rotation_speed_coefficient;
	else if(angle_to < -angle_thresshold) angle_speed = -m_rotation_speed_coefficient;
	else {
		this->SetRotation( angle );
	}
	this->SetRotationSpeed( angle_speed );
	
	const Uint8* state = SDL_GetKeyboardState(NULL);
	
	if(distance > m_max_distance_acceleration) {
		distance = m_max_distance_acceleration;
	}
	
	// this->Accelerate( -this->GetSpeed() * m_downshift_coefficient * double(GameState::deltaTime) );
	
	
	if(!GameState::input_taken && state[SDL_SCANCODE_W]) {
		
		// this->Accelerate(direction * distance * m_acceleration_speed_coefficient * double(GameState::deltaTime));
		this->SetAcceleration(direction * m_acceleration_speed_coefficient);
	} else {
		// dampening
		this->SetAcceleration(this->GetAcceleration() *  0.95);
		this->SetSpeed(this->GetSpeed() *  0.995);
	}
	
	if(!GameState::input_taken && state[SDL_SCANCODE_S]) {
		this->SetAcceleration({0,0});
		this->Accelerate( -this->GetSpeed() * m_brake_coefficient * double(GameState::deltaTime) );
	}

	Object::Process();
	
	// clip speed to m_max_speed_coefficient
	if(glm::length(this->GetSpeed()) > m_max_speed_coefficient) {
		this->SetSpeed(glm::normalize(this->GetSpeed()) * m_max_speed_coefficient);
	}
}

int fire_evt = Event::Register("fire");
void Ship::Fire() {
	const Asset::Texture& texture = GameState::asset.GetTexture("projectile.png");
	glm::dvec2 world_coord = local_to_world_coord(glm::dvec2(-texture.size.x*0.5+8.0, -m_size.y*0.5-texture.size.y*0.5-3.0));
	glm::dvec2 dir = glm::normalize(world_coord - GetPosition());
	world_coord.x -= texture.size.x*0.5;
	world_coord.y -= texture.size.y*0.5;
	Projectile projectile(texture, world_coord, glm::dvec2(0));
	
	const double acceleration_constant = m_acceleration_speed_coefficient*2;
	double speed_constant = Command::Get("projectile_speed").d;
	
	projectile.SetAcceleration(dir * acceleration_constant);
	projectile.SetSpeed(dir * speed_constant + m_speed);
	projectile.SetRotation(GetRotation());
	Network::QueueObject(&projectile);
	GameState::projectiles.push_back(projectile);
	Event::Emit(fire_evt);
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
