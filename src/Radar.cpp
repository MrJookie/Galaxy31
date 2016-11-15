#include "GameState.hpp"
#include "Ship.hpp"
#include "controls/Canvas.hpp"
#include "commands/commands.hpp"

namespace Radar {

ng::Canvas* cv_minimap = 0;
ng::Canvas* cv_minimap_ship = 0;

void Draw() {
	bool relativeRadar = true;
	if(Command::Get("radar_absolute")) {
		relativeRadar = false;
	}
	
	Ship& ship = *GameState::player;
	if(!cv_minimap)
		cv_minimap = (ng::Canvas*)GameState::gui.GetControlById("game_minimap");
	cv_minimap->Clear(0);
	cv_minimap->SetImage(std::string(TEXTURE_PATH) + std::string("hud_radar.png")); // load via asset manager?
	
	if(!cv_minimap_ship)
		cv_minimap_ship = (ng::Canvas*)GameState::gui.GetControlById("game_minimap_ship");
	cv_minimap_ship->SetImage(std::string(TEXTURE_PATH) + std::string("hud_radar_ship.png"));
	
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

// TODO: implement radar event request and radar event observer
void Init() {
	
}

}
