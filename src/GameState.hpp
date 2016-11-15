#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Asset.hpp"
#include "Camera.hpp"
//#include "Quadtree.hpp"
#include "Object.hpp"
//#include "SolidObject.hpp"
#include "Projectile.hpp"
#include "Ship.hpp"
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
	
	extern std::list<Projectile> projectiles;
	extern ng::GuiEngine gui;
	extern Asset asset;
	extern Camera camera;
	extern Ship* player;
	
	extern std::map< std::string, std::string > debug_fields;
	
	extern std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > ships;
	
	void set_gui_page(std::string page);
	
	//move to Ship
	extern int account_challenge;
	extern unsigned int user_id;
	extern std::string user_name;
	extern std::string serverPublicKeyStr;
	extern std::string clientPublicKeyStr;
	extern std::string clientPrivateKeyStr;
	extern std::array<unsigned char, 17> server_chatAESkey;
}

#endif
