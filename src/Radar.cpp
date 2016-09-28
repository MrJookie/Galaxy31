#include "GameState.hpp"
#include "Ship.hpp"
#include "controls/Canvas.hpp"
namespace Radar {

ng::Canvas* cv_minimap = 0;

void Draw() {
	Ship& ship = *GameState::player;
	if(!cv_minimap)
		cv_minimap = (ng::Canvas*)GameState::gui.GetControlById("game_minimap");
	cv_minimap->Clear(0);
	// draw my ship on radar
	int pointX = (ship.GetPosition().x / (2 * GameState::worldSize.x) * (cv_minimap->GetRect().w-5)) + (cv_minimap->GetRect().w-5)/2.0f; //-4 is for pixelsize
	int pointY = (ship.GetPosition().y / (2 * GameState::worldSize.y) * (cv_minimap->GetRect().h-5)) + (cv_minimap->GetRect().h-5)/2.0f;
	cv_minimap->SetPixelColor(0xFFFFFF00);
	cv_minimap->PutPixel(pointX, pointY);

	// draw enemy ships on radar
	for(auto& obj : GameState::ships) {
		auto& o = obj.second.first;
		
		int pointX = (o->GetPosition().x / (2 * GameState::worldSize.x) * cv_minimap->GetRect().w) + cv_minimap->GetRect().w/2.0f;
		int pointY = (o->GetPosition().y / (2 * GameState::worldSize.y) * cv_minimap->GetRect().h) + cv_minimap->GetRect().h/2.0f;
		cv_minimap->SetPixelColor(0xFFFF0000);
		cv_minimap->PutPixel(pointX, pointY);
	}
}

// TODO: implement radar event request and radar event observer
void Init() {
	
}

}
