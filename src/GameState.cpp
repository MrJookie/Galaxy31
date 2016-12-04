#include "GameState.hpp"

namespace GameState {	
	glm::vec2 windowSize;
	glm::vec2 screenMousePosition;
	glm::vec2 worldMousePosition;
	glm::vec2 worldSize(100000.0f, 100000.0f);
	int radarPerimeter = 30000; //perimeter could be set larger on mountable upgrade
	double deltaTime;
	double timeElapsed;
	float zoom;
	std::string activePage;
	std::string debug_string;
	std::map< std::string, std::string > debug_fields;
	int objectsDrawn = 0;
	
	ng::GuiEngine gui;
	Asset asset;
	Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
	
	int collision_contacts;
	
	Ship* player;
	int resource_money;
	int resource_uranium;
	int resource_energy;
	std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > enemyShips; //key: object_id?
	std::map<unsigned int, std::vector<ng::Control*>> enemyShipsHUD; //key: client_id
	std::list<Projectile> projectiles;
	std::list<Asteroid> asteroids;

	bool input_taken;
	//move to Ship
	int account_challenge;
	unsigned int client_id;
	unsigned int user_id;
	std::string user_name;
	std::string serverPublicKeyStr;
	std::string clientPublicKeyStr;
	std::string clientPrivateKeyStr;
	std::array<unsigned char, 17> server_chatAESkey;
	
	void set_gui_page(std::string page) {
		activePage = page;
		if(activePage == "login") {
			gui.GetControlById("login")->SetVisible(true);
			gui.GetControlById("register")->SetVisible(false);
			gui.GetControlById("pass_restore")->SetVisible(false);
			//gui.GetControlById("lobby")->SetVisible(false);
			gui.GetControlById("game")->SetVisible(false);
			gui.GetControlById("options")->SetVisible(false);
		} else if(activePage == "register") {
			gui.GetControlById("login")->SetVisible(false);
			gui.GetControlById("register")->SetVisible(true);
			gui.GetControlById("pass_restore")->SetVisible(false);
			//gui.GetControlById("lobby")->SetVisible(false);
			gui.GetControlById("game")->SetVisible(false);
			gui.GetControlById("options")->SetVisible(false);
		} else if(activePage == "pass_restore") {
			gui.GetControlById("login")->SetVisible(false);
			gui.GetControlById("register")->SetVisible(false);
			gui.GetControlById("pass_restore")->SetVisible(true);
			//gui.GetControlById("lobby")->SetVisible(false);
			gui.GetControlById("game")->SetVisible(false);
			gui.GetControlById("options")->SetVisible(false);
		} else if(activePage == "game") {
			gui.GetControlById("login")->SetVisible(false);
			gui.GetControlById("register")->SetVisible(false);
			gui.GetControlById("pass_restore")->SetVisible(false);
			//gui.GetControlById("lobby")->SetVisible(false);
			gui.GetControlById("game")->SetVisible(true);
			gui.GetControlById("options")->SetVisible(false);
		} else if(activePage == "game") {
			gui.GetControlById("login")->SetVisible(false);
			gui.GetControlById("register")->SetVisible(false);
			gui.GetControlById("pass_restore")->SetVisible(false);
			//gui.GetControlById("lobby")->SetVisible(false);
			gui.GetControlById("game")->SetVisible(false);
			gui.GetControlById("options")->SetVisible(true);
		}
	}

}
