#include "Collision.hpp"
#include "Quadtree.hpp"
#include <vector>
#include "EventSystem/Event.hpp"
#include "commands/commands.hpp"
#include "SolidObject.hpp"
#include "GameState.hpp"

namespace Collision {
	
	int collision_evt;
	void Init() {
		collision_evt = Event::Register("collision");
	}
	
	void Check(Quadtree* quadtree) {
		Ship& ship = *GameState::player;
		Ship* shipA;
		ship.CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
		std::vector<Object*> nearObjects;
		quadtree->QueryRectangle(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, ship.GetSize().x, ship.GetSize().y, nearObjects);
		for(auto& object : nearObjects) {
			if((SolidObject*)&ship == (SolidObject*)object) continue;
			if(object->GetType() == object_type::ship) {
				
			}
			if(Command::Get("aabb"))
			quadtree->DrawRect(object->GetPosition().x - object->GetSize().x/2, object->GetPosition().y - object->GetSize().y/2, object->GetSize().x, object->GetSize().y, glm::vec4(1, 1, 1, 1));
			
			if(ship.Collides((SolidObject*)object)) {
				Event::Emit(collision_evt, &ship, object);
				ship.CollisionHullColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
			}
			
			if(((SolidObject*)object)->NodePtr == nullptr) continue;
			
			std::vector<Object*> objcts;
			//((SolidObject*)object)->NodePtr->GetObjectsInNode(objcts);
			quadtree->QueryRectangle(object->GetPosition().x - object->GetSize().x/2, object->GetPosition().y - object->GetSize().y/2, object->GetSize().x, object->GetSize().y, objcts);
			for(auto& object2 : objcts) {
				if(Command::Get("aabb"))
				quadtree->DrawRect(object2->GetPosition().x - object2->GetSize().x/2, object2->GetPosition().y - object2->GetSize().y/2, object2->GetSize().x, object2->GetSize().y, glm::vec4(0, 0, 1, 1));
			}
		}
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
