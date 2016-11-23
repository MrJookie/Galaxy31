#include "GameState.hpp"
#include "Ship.hpp"
#include "controls/Canvas.hpp"
#include "commands/commands.hpp"

namespace HUD {

ng::Canvas* cv_minimap = 0;
ng::Canvas* cv_minimap_ship = 0;
ng::Label* lb_game_bar_basic = 0;
ng::Label* tb_game_ship_armor = 0;
ng::Label* tb_game_ship_armor_enemy = 0; //dynamically create

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
	
	cv_minimap->Clear(0);

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
	
	cv_minimap->SetPixelColor(0xFFFFFF00);
	cv_minimap->PutPixel(pointX, pointY);
	
	glm::vec2 myShipPosition = ship.GetPosition();

	// draw enemy ships on radar
	for(auto& enemyObj : GameState::ships) {
		auto& enemyShip = enemyObj.second.first;
		
		if(relativeRadar) {
			glm::dvec2 relativePosition = glm::dvec2(enemyShip->GetPosition().x - ship.GetPosition().x, enemyShip->GetPosition().y - ship.GetPosition().y);
			
			float enemyDistance = glm::length(relativePosition);
			if(enemyDistance < GameState::radarPerimeter.x) { //radius
				pointX = (relativePosition.x / (2 * GameState::radarPerimeter.x) * (cv_minimap->GetRect().w-4)) + (cv_minimap->GetRect().w-4)/2.0f;
				pointY = (relativePosition.y / (2 * GameState::radarPerimeter.y) * (cv_minimap->GetRect().h-4)) + (cv_minimap->GetRect().h-4)/2.0f;
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
}

void BarResources() {
	if(!lb_game_bar_basic) {
		lb_game_bar_basic = (ng::Label*)GameState::gui.GetControlById("game_bar_basic");
		lb_game_bar_basic->SetImage(std::string(TEXTURE_PATH) + std::string("hud_bar_basic.png"));
	}
}

void ShipBillboards() {
	Ship& ship = *GameState::player;
	
	if(!tb_game_ship_armor) {
		tb_game_ship_armor = (ng::Label*)GameState::gui.GetControlById("game_ship_armor");
		tb_game_ship_armor->SetText(GameState::user_name);
	}
	
	float offset = GameState::zoom*25.0f;
	
	glm::ivec4 myShipWorldSpace(ship.GetPosition().x - ship.GetSize().x/2.0, ship.GetPosition().y - ship.GetSize().y/2.0 - offset, 0.0f, 1.0f);
	glm::ivec2 myShipScreenSpace = GameState::camera.worldToScreen(myShipWorldSpace, GameState::windowSize, GameState::camera.GetViewMatrix(), GameState::camera.GetProjectionMatrix());
			
	tb_game_ship_armor->SetPosition(myShipScreenSpace.x, myShipScreenSpace.y);
	
	//or just +offsets
	//tb_game_ship_armor->SetPosition(GameState::windowSize.x/2.0f, GameState::windowSize.y/2.0f);
	
	for(auto& enemyObj : GameState::ships) {
		auto& enemyShip = enemyObj.second.first;

		//dynamically create and dealloc?
		if(!tb_game_ship_armor_enemy) {
			tb_game_ship_armor_enemy = (ng::Label*)GameState::gui.GetControlById("game_ship_armor_enemy");
			/*
			tb_game_ship_armor_enemy = (ng::Label*)GameState::gui.ControlManager::CreateControl("game_ship_armor_enemy");
			
			GameState::gui.AddControl(tb_game_ship_armor_enemy);
			*/
			
			tb_game_ship_armor_enemy->SetText(enemyShip->name.data());
		}
		
		glm::ivec4 enemyShipWorldSpace(enemyShip->GetPosition().x - enemyShip->GetSize().x/2.0, enemyShip->GetPosition().y - enemyShip->GetSize().y/2.0 - offset, 0.0f, 1.0f);
		glm::ivec2 enemyShipScreenSpace = GameState::camera.worldToScreen(enemyShipWorldSpace, GameState::windowSize, GameState::camera.GetViewMatrix(), GameState::camera.GetProjectionMatrix());
			
		tb_game_ship_armor_enemy->SetPosition(enemyShipScreenSpace.x, enemyShipScreenSpace.y);
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
