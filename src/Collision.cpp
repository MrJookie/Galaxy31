#include "Collision.hpp"
#include "Quadtree.hpp"
#include <vector>
#include "EventSystem/Event.hpp"
#include "commands/commands.hpp"
#include "SolidObject.hpp"
#include "GameState.hpp"
#include <iostream>

namespace Collision {
		
	int collision_evt;
	void Init() {
		collision_evt = Event::Register("collision");
	}
			
	void CheckProjectilesObject(Quadtree* quadtree) {
		Object& myShip = *GameState::player;
		
		std::vector<Object*> projectiles;
		quadtree->QueryRectangle(myShip.GetPosition().x - GameState::windowSize.x/2*GameState::zoom, myShip.GetPosition().y - GameState::windowSize.y/2*GameState::zoom, GameState::windowSize.x*GameState::zoom, GameState::windowSize.y*GameState::zoom, projectiles);
		for(auto& projectile : projectiles) {
			if(projectile->GetType() != object_type::projectile) continue;
						
			std::vector<Object*> objects;
			quadtree->QueryRectangle(projectile->GetPosition().x - projectile->GetSize().x/2, projectile->GetPosition().y - projectile->GetSize().y/2, projectile->GetSize().x, projectile->GetSize().y, objects);
			for(auto& object : objects) {
				// if(ship->GetType() != object_type::ship) continue;
				if(object->GetType() != object_type::ship && object->GetType() != object_type::asteroid || projectile->GetOwner() == object->GetId()) continue;
				
				if(((SolidObject*)projectile)->Collides((SolidObject*)object) || ((SolidObject*)projectile)->CollidesProjectileRay((SolidObject*)object)) {
					Event::Emit(collision_evt, object, projectile);
					
					//std::cout << "projectile collided an object" << std::endl;
				}
			}
		}
	}
	
	void CheckShipObject(Quadtree* quadtree) {
		bool had_collision = false;
		
		Ship& ship = *GameState::player;
		
		ship.CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
		std::vector<Object*> nearObjects;
		quadtree->QueryRectangle(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, ship.GetSize().x, ship.GetSize().y, nearObjects);
		for(auto& object : nearObjects) {
			if(object->GetType() != object_type::ship && object->GetType() != object_type::asteroid) continue;
			if((SolidObject*)object == (SolidObject*)&ship) continue;

			if(Command::Get("aabb"))
				quadtree->DrawRect(object->GetPosition().x - object->GetSize().x/2, object->GetPosition().y - object->GetSize().y/2, object->GetSize().x, object->GetSize().y, glm::vec4(1, 1, 1, 1));
			
			((SolidObject*)object)->CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
			
			//if(((SolidObject*)object)->NodePtr == nullptr) continue;
			
			if(((SolidObject*)&ship)->Collides((SolidObject*)object)) {
					((SolidObject*)&ship)->CollisionHullColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
					
					glm::vec2 myShipPosition = ((SolidObject*)&ship)->GetPosition();
					glm::vec2 enemyShipPosition = ((SolidObject*)object)->GetPosition();
					
					//spawn
					//~~~~NEEDS FIX, MAY CAUSE NaN!!
					if(myShipPosition.x == enemyShipPosition.x && myShipPosition.y == enemyShipPosition.y) {
						return;
					} else {
						glm::vec2 reflectDirection = glm::normalize(glm::dvec2(myShipPosition.x - enemyShipPosition.x, myShipPosition.y - enemyShipPosition.y));
					
						//float dist = glm::length(glm::dvec2(enemyShipPosition.x - myShipPosition.x, enemyShipPosition.y - myShipPosition.y));
						
						float speedLen = glm::length( ((SolidObject*)&ship)->GetSpeed() );
						float rayLen = speedLen > 100.0f ? std::min(speedLen, 500.0f) : 100.0f;
						float speedMul = 0.1f;
						
						/*
						//colliding for too long, prevention against penetration
						if(GameState::collision_contacts > 0) {
							//rayLen = 60.0f;
						}
						*/
						
						((SolidObject*)&ship)->SetAcceleration({0,0});
						((SolidObject*)&ship)->SetSpeed(glm::dvec2( ((SolidObject*)&ship)->GetSpeed().x*speedMul + reflectDirection.x * rayLen, ((SolidObject*)&ship)->GetSpeed().y*speedMul + reflectDirection.y * rayLen));
						
						Event::Emit(collision_evt, (Object*)&ship, object);
						
						//std::cout << "ship collided an object" << std::endl;
					}
					
					had_collision = true;
				}

			/*
			std::vector<Object*> objcts;
			//((SolidObject*)object)->NodePtr->GetObjectsInNode(objcts);
			quadtree->QueryRectangle(object->GetPosition().x - object->GetSize().x/2, object->GetPosition().y - object->GetSize().y/2, object->GetSize().x, object->GetSize().y, objcts);
			for(auto& object2 : objcts) {
				if(object2->GetType() != object_type::ship) continue;
				if(object == object2) continue;
				if((SolidObject*)&ship != (SolidObject*)object2) continue;
				
				if(Command::Get("aabb"))
					quadtree->DrawRect(object2->GetPosition().x - object2->GetSize().x/2, object2->GetPosition().y - object2->GetSize().y/2, object2->GetSize().x, object2->GetSize().y, glm::vec4(0, 0, 1, 1));
			
				
			}
			*/
		}
		
		if(had_collision) {
			GameState::collision_contacts++;
		} else {
			GameState::collision_contacts = 0;
		}
	}
	
	void Check(Quadtree* quadtree) {
		CheckProjectilesObject(quadtree);
		CheckShipObject(quadtree);
	}
	
	void WorldBoundary() {
		Ship& ship = *GameState::player;
		// world boundaries
		if(ship.GetPosition().x > GameState::worldSize.x) {
			ship.SetSpeed(glm::vec2(0, ship.GetSpeed().y));
			ship.SetPosition(glm::vec2(GameState::worldSize.x, ship.GetPosition().y));
		}

		if(ship.GetPosition().x < -GameState::worldSize.x) {
			ship.SetSpeed(glm::vec2(0, ship.GetSpeed().y));
			ship.SetPosition(glm::vec2(-GameState::worldSize.x, ship.GetPosition().y));
		}
		
		if(ship.GetPosition().y > GameState::worldSize.y) {
			ship.SetSpeed(glm::vec2(ship.GetSpeed().x, 0));
			ship.SetPosition(glm::vec2(ship.GetPosition().x, GameState::worldSize.y));
		}

		if(ship.GetPosition().y < -GameState::worldSize.y) {
			ship.SetSpeed(glm::vec2(ship.GetSpeed().x, 0));
			ship.SetPosition(glm::vec2(ship.GetPosition().x, -GameState::worldSize.y));
		}
	}
	
}
