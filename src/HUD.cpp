#include "GameState.hpp"
#include "Ship.hpp"
#include "controls/Canvas.hpp"
#include "commands/commands.hpp"

namespace HUD {

ng::Canvas* cv_minimap = 0;
ng::Canvas* cv_minimap_ship = 0;
ng::Label* lb_game_bar_basic = 0;
ng::Label* lb_game_ship_nickname = 0;
ng::Label* lb_game_bar_basic_money = 0;

void Radar() {
	bool relativeRadar = true;
	if(Command::Get("radar_absolute")) {
		relativeRadar = false;
	}
	
	Ship& ship = *GameState::player;
	if(!cv_minimap) {
		cv_minimap = (ng::Canvas*)GameState::gui.GetControlById("game_minimap");
		cv_minimap->SetImage(std::string(TEXTURE_PATH) + std::string("hud_radar.png")); // load via asset manager?
	}
	
	cv_minimap->Clear(0x00000000);

	if(!cv_minimap_ship) {
		cv_minimap_ship = (ng::Canvas*)GameState::gui.GetControlById("game_minimap_ship");
		cv_minimap_ship->SetImage(std::string(TEXTURE_PATH) + std::string("hud_radar_ship.png"));
	}
	
	int pointX;
	int pointY;
	
	// draw my ship on radar
	if(relativeRadar) {
		pointX = (cv_minimap->GetRect().w-4)/2.0f; //-4 is for pixelsize
		pointY = (cv_minimap->GetRect().h-4)/2.0f;
	} else {
		pointX = (ship.GetPosition().x / (2 * GameState::worldSize.x) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f; //-4 is for pixelsize
		pointY = (ship.GetPosition().y / (2 * GameState::worldSize.y) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
	}
	
	/*
	cv_minimap->SetPixelColor(0xFFFFFF00);
	cv_minimap->PutPixel(pointX, pointY);
	*/
	
	glm::vec2 myShipPosition = ship.GetPosition();

	// draw enemy ships on radar
	for(auto& enemyObj : GameState::enemyShips) {
		auto& enemyShip = enemyObj.second.first;
		
		if(relativeRadar) {
			glm::vec2 relativePosition = glm::vec2(enemyShip->GetPosition().x - ship.GetPosition().x, enemyShip->GetPosition().y - ship.GetPosition().y);
			
			float objectDistance = glm::length(relativePosition);
			if(objectDistance < GameState::radarPerimeter) { //radius
				pointX = (relativePosition.x / (2 * GameState::radarPerimeter) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f;
				pointY = (relativePosition.y / (2 * GameState::radarPerimeter) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
			
			//if(cv_minimap->GetPixel().a != 0x00) { // if pixel is not transparent (get from SetImage)
				cv_minimap->SetPixelColor(0xFFFF0000);
				cv_minimap->PutPixel(pointX, pointY);
			}
		} else {
			pointX = (enemyShip->GetPosition().x / (2 * GameState::worldSize.x) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f;
			pointY = (enemyShip->GetPosition().y / (2 * GameState::worldSize.y) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
			
			cv_minimap->SetPixelColor(0xFFFF0000);
			cv_minimap->PutPixel(pointX, pointY);
		}
	}
	
	// draw asteroids on radar
	for(auto& asteroid : GameState::asteroids) {
		
		if(relativeRadar) {
			glm::vec2 relativePosition = glm::vec2(asteroid.GetPosition().x - ship.GetPosition().x, asteroid.GetPosition().y - ship.GetPosition().y);
			
			float objectDistance = glm::length(relativePosition);
			if(objectDistance < GameState::radarPerimeter) { //radius
				pointX = (relativePosition.x / (2 * GameState::radarPerimeter) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f;
				pointY = (relativePosition.y / (2 * GameState::radarPerimeter) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
				
				cv_minimap->SetPixelColor(0xFF777777);
				cv_minimap->PutPixel(pointX, pointY);
			}
		} else {
			pointX = (asteroid.GetPosition().x / (2 * GameState::worldSize.x) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f;
			pointY = (asteroid.GetPosition().y / (2 * GameState::worldSize.y) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
			
			cv_minimap->SetPixelColor(0xFF777777);
			cv_minimap->PutPixel(pointX, pointY);
		}
	}
	
	cv_minimap->RefreshTexture();
}

void BarResources() {
	if(!lb_game_bar_basic) {
		lb_game_bar_basic = (ng::Label*)GameState::gui.GetControlById("game_bar_basic");
		lb_game_bar_basic->SetImage(std::string(TEXTURE_PATH) + std::string("hud_bar_basic.png"));
	}
	
	if(!lb_game_bar_basic_money) {
		lb_game_bar_basic_money = (ng::Label*)GameState::gui.GetControlById("game_bar_basic_money");
	}
	
	lb_game_bar_basic_money->SetText(std::to_string(GameState::resource_money));
}

void ShipBillboards() {
	Ship& ship = *GameState::player;
	
	if(!lb_game_ship_nickname) {
		lb_game_ship_nickname = (ng::Label*)GameState::gui.GetControlById("game_ship_nickname");
		lb_game_ship_nickname->SetText(GameState::user_name);
	}
		
	float offset = GameState::zoom*25.0f;
	
	glm::ivec4 myShipWorldSpace(std::floor(ship.GetPosition().x - ship.GetSize().x/2), std::floor(ship.GetPosition().y - ship.GetSize().y/2 - offset), 0.0f, 1.0f);
	glm::ivec2 myShipScreenSpace = GameState::camera.worldToScreen(myShipWorldSpace, GameState::windowSize, GameState::camera.GetViewMatrix(), GameState::camera.GetProjectionMatrix());
			
	lb_game_ship_nickname->SetPosition(myShipScreenSpace.x, myShipScreenSpace.y);
	
	//or just +offsets
	//lb_game_ship_armor->SetPosition(GameState::windowSize.x/2.0f, GameState::windowSize.y/2.0f);
	
	for(auto& enemyObj : GameState::enemyShips) {
		auto& enemyShip = enemyObj.second.first;
		unsigned int client_id = enemyShip->GetId();
		
		//if HUD not created for enemy ship yet
		if(GameState::enemyShipsHUD.count(client_id) == 0) {
			ng::Label* lb_enemy_ship_nickname = (ng::Label*)GameState::gui.CreateControl("label");
			lb_enemy_ship_nickname->SetId("enemy_ship_nickname_" + std::to_string(enemyShip->GetOwner()));
			lb_enemy_ship_nickname->SetStyle("rect", "0,0,265,29");
			lb_enemy_ship_nickname->SetText(enemyShip->name.data());
			
			GameState::gui.AddControl(lb_enemy_ship_nickname); //removed in Network.cpp
						
			std::vector<ng::Control*> controls;
			controls.push_back(lb_enemy_ship_nickname);
			GameState::enemyShipsHUD[client_id] = controls;
		}
		
		glm::ivec4 enemyShipWorldSpace(std::floor(enemyShip->GetPosition().x - enemyShip->GetSize().x/2), std::floor(enemyShip->GetPosition().y - enemyShip->GetSize().y/2 - offset), 0.0f, 1.0f);
		glm::ivec2 enemyShipScreenSpace = GameState::camera.worldToScreen(enemyShipWorldSpace, GameState::windowSize, GameState::camera.GetViewMatrix(), GameState::camera.GetProjectionMatrix());
		
		//0 = nickname
		GameState::enemyShipsHUD[client_id][0]->SetPosition(enemyShipScreenSpace.x, enemyShipScreenSpace.y);
	}
}

void Draw() {
	Radar();
	BarResources();
	ShipBillboards();
}

// TODO: implement radar event request and radar event observer
void Init() {

}

}
