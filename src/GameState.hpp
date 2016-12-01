#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Asset.hpp"
#include "Camera.hpp"
//#include "Quadtree.hpp"
#include "Object.hpp"
//#include "SolidObject.hpp"
#include "Projectile.hpp"
#include "Ship.hpp"
#include "Asteroid.hpp"
#include <vector>
#include <utility>
#include <queue>
#include <map>
#include <list>

#include "Gui.hpp"
#include "Control.hpp"
#include "controls/Button.hpp"
#include "controls/ScrollBar.hpp"
// #include "controls/Container.hpp"
#include "controls/ComboBox.hpp"
// #include "controls/GridContainer.hpp"
#include "controls/TextBox.hpp"
#include "controls/RadioButton.hpp"
#include "controls/ListBox.hpp"
#include "controls/Label.hpp"
// #include "controls/TrackBar.hpp"
#include "controls/Canvas.hpp"
// #include "controls/CheckBox.hpp"
// #include "controls/WidgetMover.hpp"
#include "controls/Terminal.hpp"
#include "controls/Form.hpp"
#include "common/SDL/Drawing.hpp"

#define SHADER_PATH "Assets/Shaders/"
#define TEXTURE_PATH "Assets/Textures/"
#define MUSIC_PATH "Assets/Music/"
#define SOUND_PATH "Assets/Sound/"

namespace GameState {
	extern glm::vec2 windowSize;
	extern glm::vec2 screenMousePosition;
	extern glm::vec2 worldMousePosition;
	extern glm::vec2 worldSize;
	extern glm::vec2 radarPerimeter;
	extern double deltaTime;
	extern double timeElapsed;
	extern float zoom;
	extern std::string activePage;
	extern bool input_taken;
	
	extern int objectsDrawn;
	extern std::string debug_string;
	extern std::map< std::string, std::string > debug_fields;
	
	extern ng::GuiEngine gui;
	extern Asset asset;
	extern Camera camera;
	
	extern Ship* player;
	extern int resource_money;
	extern int resource_uranium;
	extern int resource_energy;
	//extern int level_exp; //set scale, 0-99 = lvl1, 100-1000 = lvl2 and so on, so if user has 150exp we know its lvl1 +50exp out of 1000 to next lvl (level details should be hardcoded)
	extern std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > enemyShips; //key: object_id
	extern std::map<unsigned int, std::vector<ng::Control*>> enemyShipsHUD; //key: client_id
	extern std::list<Projectile> projectiles;
	extern std::list<Asteroid> asteroids;
	
	void set_gui_page(std::string page);
	
	//move to Ship
	extern int account_challenge;
	extern unsigned int client_id;
	extern unsigned int user_id;
	extern std::string user_name;
	extern std::string serverPublicKeyStr;
	extern std::string clientPublicKeyStr;
	extern std::string clientPrivateKeyStr;
	extern std::array<unsigned char, 17> server_chatAESkey;
}

#endif
