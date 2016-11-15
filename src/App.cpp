#include "App.hpp"
#include "Network.hpp"
#include "GameState.hpp"
#include "Quadtree.hpp"
#include "commands/commands.hpp"
#include "commands/bind.hpp"
#include "server/network.hpp"
#include "EventSystem/Event.hpp"
#include "Collision.hpp"
#include "Radar.hpp"
#include <sstream>
#include <algorithm>

using Commands::Arg;
using ng::Control;
using ng::Button;
using ng::TextBox;
// using ng::TextBox;

bool toggleMouseRelative = false;
bool toggleFullscreen = false;
bool toggleWireframe = false;

int skipMouseResolution = 0;
bool wait_for_packets = false;
bool isFiring = false;
std::vector<Ship*> ships;
ng::Canvas *cv_minimap;
ng::TextBox *tb_debug;
TextBox* tb_game_user_name;
int tick_id;

App::App() {
	m_initialWindowSize = glm::vec2(1024, 768);
    this->setWindowSize(m_initialWindowSize);

    m_ticks_previous = SDL_GetTicks();
    m_ticks_current = 0;
    m_frames_current = 0;
    m_frames_elapsed = 0;

    m_delta_time = 0;
    m_ticks_then = 0;

    m_chrono_start = std::chrono::high_resolution_clock::now();
    m_chrono_elapsed = 0;
    
   
    this->generate_RSA_keypair();
}

App::~App() {
	GameState::asset.FreeAssets();
    Mix_CloseAudio();
    Mix_Quit();
    Network::cleanup();
    NetworkChat::cleanup();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

namespace Timer {
	void Init();
}

void App::init() {
	
	tick_id = Event::Register("tick");
	Timer::Init();
    Network::initialize();
    NetworkChat::initialize();
    // Network::connect("89.177.76.215", SERVER_PORT);
    
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::string("Failed to initialize SDL: ") + SDL_GetError();
    }

    window = SDL_CreateWindow("Galaxy31", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->getWindowSize().x, this->getWindowSize().y, SDL_WINDOW_OPENGL);
    if(window == nullptr) {
        throw std::string("Failed to create window: ") + SDL_GetError();
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    glContext = SDL_GL_CreateContext(window);
    if(glContext == nullptr) {
        throw std::string("Failed to create GLContext: ") + SDL_GetError();
    }

    SDL_GL_SetSwapInterval(1);
    SDL_GL_MakeCurrent(window, glContext);

    glewExperimental = GL_TRUE;
    glewInit();

    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

   
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetWindowGrab(window, SDL_TRUE);

    if(toggleMouseRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    
    init_audio();
    
    //load all textures here
    GameState::asset.LoadTexture("ship_01_skin.png");
    GameState::asset.LoadTextureHull("ship_01_skin_collision.png");
    
    //(projectile.png loads on GetTexture in Ship.cpp)
    GameState::asset.LoadTextureHull("projectile_collision.png");
    
    GameState::asset.LoadTexture("propulsion.png");

    GameState::asset.LoadShader("background.vs", "background.fs");
    GameState::asset.LoadShader("sprite.vs", "sprite.fs");
    GameState::asset.LoadShader("shader1.vs", "shader1.fs");
    
    //gui
    GameState::gui = ng::GuiEngine(this->getWindowSize().x, this->getWindowSize().y);
	GameState::gui.SetDefaultFont("Assets/Fonts/DroidSansMono.ttf");
	GameState::gui.LoadXml("Assets/gui.xml");
	GameState::gui.ApplyAnchoring();
	
	tb_debug = (TextBox*)GameState::gui.GetControlById("game_debug");
	// cv_minimap = (Canvas*)GameState::gui.GetControlById("game_minimap");
	
	GameState::set_gui_page("login");

	Button &bt_login_submit = *((Button*)GameState::gui.GetControlById("login_submit"));
	bt_login_submit.SubscribeEvent(Button::event::click, [&](Control* c) {
		//Button* bt = (Button*)c;
		
		TextBox* tb_login_status = (TextBox*)GameState::gui.GetControlById("login_status");
		TextBox* tb_login_email = (TextBox*)GameState::gui.GetControlById("login_email");
		TextBox* tb_login_password = (TextBox*)GameState::gui.GetControlById("login_password");

		tb_login_status->SetText("Logging in...");
		tb_login_status->SetVisible(true);

		Network::SendAuthentication(tb_login_email->GetText(), tb_login_password->GetText());
	});
	
	Button &bt_pass_restore_submit = *((Button*)GameState::gui.GetControlById("pass_restore_submit"));
	bt_pass_restore_submit.SubscribeEvent(Button::event::click, [&](Control* c) {
		TextBox* tb_pass_restore_email = (TextBox*)GameState::gui.GetControlById("pass_restore_email");
		
		if(this->server_doPassRestore(tb_pass_restore_email->GetText())) {
			GameState::gui.GetControlById("pass_restore_ok")->SetVisible(true);
		}
	});
	
	Button &bt_register_submit = *((Button*)GameState::gui.GetControlById("register_submit"));
	bt_register_submit.SubscribeEvent(Button::event::click, [&](Control* c) {
		TextBox* tb_register_status = (TextBox*)GameState::gui.GetControlById("register_status");
		TextBox* tb_register_email = (TextBox*)GameState::gui.GetControlById("register_email");
		TextBox* tb_register_username = (TextBox*)GameState::gui.GetControlById("register_username");
		TextBox* tb_register_password = (TextBox*)GameState::gui.GetControlById("register_password");
		TextBox* tb_register_password_verify = (TextBox*)GameState::gui.GetControlById("register_password_verify");

		tb_register_status->SetText("Creating account...");
		tb_register_status->SetVisible(true);
		
		if(tb_register_password->GetText() == tb_register_password_verify->GetText()) {
			Network::SendRegistration(tb_register_email->GetText(), tb_register_username->GetText(), tb_register_password->GetText());
		} else {
			tb_register_status->SetText("Error! Passwords differ...");
		}
	});
	
	tb_game_user_name = (TextBox*)GameState::gui.GetControlById("game_user_name");
	
	Button &bt_login_register = *((Button*)GameState::gui.GetControlById("login_register"));
	bt_login_register.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::set_gui_page("register");
	});
	
	Button &bt_login_pass_restore = *((Button*)GameState::gui.GetControlById("login_pass_restore"));
	bt_login_pass_restore.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::set_gui_page("pass_restore");
	});
	
	Button &bt_register_login = *((Button*)GameState::gui.GetControlById("register_login"));
	bt_register_login.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::set_gui_page("login");
	});
	
	Button &bt_pass_restore_login = *((Button*)GameState::gui.GetControlById("pass_restore_login"));
	bt_pass_restore_login.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::set_gui_page("login");
	});

	ng::Terminal &tm_game_chat = *((ng::Terminal*)GameState::gui.GetControlById("game_terminal"));

	tm_game_chat.SubscribeEvent(ng::Terminal::event::command, [&](Control* c) {
		ng::Terminal* t = (ng::Terminal*)c;
		if(t->GetLastCommand().size() > 0) {
			if(t->GetLastCommand()[0] == '/') {
				try {
					Commands::Arg a = Command::Execute(t->GetLastCommand().substr(1));
					if(a.type != Commands::Arg::t_void)
						t->WriteLog(a);
				} catch(...) { std::cout << "command not found" << std::endl; }
			} else {
				t->WriteLog(GameState::user_name + ": " + t->GetLastCommand() + "^w");
				NetworkChat::SendChatMessage("", t->GetLastCommand());
			}
		}
		t->Unselect();
		GameState::gui.Activate(0);
	});
	
	init_commands();
		
	Ship::Chassis chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
    GameState::player = new Ship(glm::vec2(0, 0), 0.0, chassis);;
    
    m_quadtree = new Quadtree ( -GameState::worldSize.x, GameState::worldSize.x, -GameState::worldSize.y, GameState::worldSize.x, 6 ) ;
	//m_quadtree->Resize();
   
    /*
    for(int x = 0; x < 20; ++x) {
		for(int y = 0; y < 20; y++) {
			Ship* ship = new Ship({x*1000, y*1000}, 0.0, chassis);
			ships.push_back(ship);
		}
	}
	*/
	
	Command::LoadFromFile("galaxy31.cfg");
	Command::LoadFromFile("user.cfg");
	Network::connect(Command::GetString("server_ip").c_str(), SERVER_PORT);
	
	Collision::Init();
	
	// tm_game_chat.SetImage("Assets/Textures/propulsion.png");
	
	Event::Listen("collision", [&](Object* obj1, Object* obj2) {
		
		if(obj2->GetType() == object_type::projectile) {
			unsigned int user_id = (obj2->GetOwner() == GameState::player->GetId()) ? GameState::player->GetOwner() : GameState::ships[obj2->GetOwner()].first->GetOwner();
			std::cout << "COLLISION !! projectile user_id: " << user_id << " hit ship player user_id: " << obj1->GetOwner() << std::endl;
			((Projectile*)obj2)->Destroy();
			if(obj1->GetType() == object_type::ship) { 
				if(obj1 == GameState::player) {
					Command::Execute("im_hit");
				} else {
					Command::Execute("enemy_hit");
				}
			}
		}
		
		if(obj2->GetType() == object_type::ship) {
			//unsigned int user_id = (obj2->GetOwner() == GameState::player->GetId()) ? GameState::player->GetOwner() : GameState::ships[obj2->GetOwner()].first->GetOwner();
			std::cout << "COLLISION !! ship user_id: " << obj1->GetOwner() << " hit ship player user_id: " << obj2->GetOwner() << std::endl;
		}
		
	});
	
	Event::Listen("disconnected", [](){
		cout << "You are disconnected from server" << endl;
		exit(-1);
	});
}

void App::main_loop() {
	
	Ship& ship = *GameState::player;
	ng::Terminal &tm_game_chat = *((ng::Terminal*)GameState::gui.GetControlById("game_terminal"));
	Ship::Chassis chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
	int lst = Event::Listen("timer", []() {
		cout << "timer test each 5 seconds \n";
	}, 5.0);
	Event::StopListening(lst);
	
	while(m_running) {
        this->loop();

        SDL_Event e;
		Event::Emit(tick_id, GameState::deltaTime);
		GameState::input_taken = GameState::gui.GetActiveControl() != nullptr;
        while(SDL_PollEvent(&e)) {

            if(e.type == SDL_QUIT) {
                m_running = false;
            } else if(e.type == SDL_KEYDOWN) {
				if(!GameState::input_taken) {
					if(m_bind.OnEvent(e).i != 0)
						continue;
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							m_running = false;
						break;
						case SDLK_SLASH:
							tm_game_chat.Focus();
							break;
					}
				}
            } else if(e.type == SDL_MOUSEBUTTONDOWN && GameState::gui.GetSelectedControl() == nullptr) {
				
				if(e.button.button == SDL_BUTTON_LEFT) {
					if(Command::Get("place_ship")) {
						Ship* ship = new Ship(this->getWorldMousePosition(), 0.0, chassis);
						ships.push_back(ship);
					}
					
					isFiring = true;
				} else {
					ship.Stabilizers();
				}
            } else if(e.type == SDL_MOUSEBUTTONUP && GameState::gui.GetSelectedControl() == nullptr) {
				if(e.button.button == SDL_BUTTON_LEFT) {
					isFiring = false;
				} else {
					ship.Stabilizers();
				}
            } else if(e.type == SDL_MOUSEWHEEL) {
				this->setZoom((this->getZoom() - e.wheel.y) > 30 ? this->getZoom() : std::max(this->getZoom() - e.wheel.y, 1.0f));
            }
            
            
            // code completion
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB && tm_game_chat.IsSelected() && tm_game_chat.GetText()[0] == '/') {
				
				std::string text = tm_game_chat.GetText();
				std::vector<std::string> vec = Command::Search(text, text.size()-1, 5);
				if(vec.size() > 1) {
					for(auto& s : vec) {
						tm_game_chat.AppendLog("^c" + text + "^w" + s);
					}
					tm_game_chat.AppendLog("-------");
				}
				text += Command::Complete(text, text.size()-1);
				if(!text.empty())
					tm_game_chat.SetText(text);
				
			} else {
				
				GameState::gui.OnEvent(e);
			}
        }
		
        glViewport(0, 0, this->getWindowSize().x, this->getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        SDL_PumpEvents();
        //const Uint8* state = SDL_GetKeyboardState(NULL);

        int mousePositionX, mousePositionY;
        SDL_GetRelativeMouseState(&mousePositionX, &mousePositionY);
        this->setScreenMousePosition(glm::vec2(mousePositionX, mousePositionY));

        if(skipMouseResolution > 0 && (this->getScreenMousePosition().x != 0 || this->getScreenMousePosition().y != 0)) {
            skipMouseResolution--;
        } else {
            SDL_GetMouseState(&mousePositionX, &mousePositionY);
            this->setScreenMousePosition(glm::vec2(mousePositionX, mousePositionY));

            this->setWorldMousePosition(
				glm::vec2(
					((float)this->getScreenMousePosition().x - this->getWindowSize().x * 0.5) * m_zooming + GameState::camera.GetPosition().x,
					((float)this->getScreenMousePosition().y - this->getWindowSize().y * 0.5) * m_zooming + GameState::camera.GetPosition().y
				)
			);
        }
        
        GameState::windowSize = this->getWindowSize();
        GameState::screenMousePosition = this->getScreenMousePosition();
        GameState::worldMousePosition = this->getWorldMousePosition();
        GameState::deltaTime = this->getDeltaTime();
        GameState::timeElapsed = this->getTimeElapsed();
        GameState::zoom = this->getZoom();
        GameState::objectsDrawn = 0;
        
        Network::handle_events(5);

		// draw space background
        glUseProgram(GameState::asset.GetShader("background.vs").id);

        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        GLfloat position[] = {
		   -1.0,   1.0,
		   -1.0,  -1.0,
			1.0,  -1.0,
			1.0,   1.0,
		};
		
		GLuint indices[] = {
			0, 1, 3,
			1, 2, 3,
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("background.vs").id, "shipPosition"), ship.GetPosition().x, -ship.GetPosition().y);
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("background.vs").id, "windowSize"), this->getWindowSize().x, this->getWindowSize().y);
        glUniform1f(glGetUniformLocation(GameState::asset.GetShader("background.vs").id, "time"), this->getTimeElapsed());

        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
        
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);

        glUseProgram(0);
        //
        
        if(GameState::activePage == "game") {
			game_loop();
		}
        
        GameState::gui.Render();

        SDL_GL_SwapWindow(window);

        // SDL_Delay(16);
    }
}

auto firing_tp = std::chrono::steady_clock::now();
void App::game_loop() {
	GameState::debug_string = "";
	Ship& ship = *GameState::player;
	NetworkChat::handle_events(5);
		
	Radar::Draw();
	
	Collision::WorldBoundary();
	//
	
	//Network::handle_events(5);
	ship.Process();
	
	for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
		it->Process();
	}
	
	Network::Process();
	
	if(isFiring) {
		auto delta = std::chrono::steady_clock::now() - firing_tp;
		if(std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() > Command::Get("rocket_milliseconds")) {
			ship.Fire();
			firing_tp = std::chrono::steady_clock::now();
		}
	}

	GameState::camera.SetPosition( glm::vec3(ship.GetPosition(), 0) );
	glm::mat4 projection = glm::ortho(-(float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().y*this->getZoom()*0.5, -(float)this->getWindowSize().y*this->getZoom()*0.5);
	glm::mat4 view = GameState::camera.GetViewMatrix();
	GameState::camera.SetProjection(projection);
	GameState::camera.SetView(view);
	
	Network::SendOurState();
	
	// add projectiles to m_quadtree, Update (lifetime) could be move here, now is in 
	for(auto& projectile : GameState::projectiles) {
		m_quadtree->AddObject(&projectile);
			
		projectile.Update(); //update lifetime, has to be separated from Draw(), otherwise quadtree will update lifetime of queried projectiles
		projectile.GetSprite()->RemoveFromDrawing(); //remove projectile from drawing, new draw will be filled by quadtree on next frame
		
		projectile.UpdateHullVertices(GameState::asset.GetTextureHull("projectile_collision.png").vertices);
		if(Command::Get("collisionhull")) {
			projectile.RenderCollisionHull();
			projectile.RenderProjectileRay();
		}
	}
	
	// add multiplayer enemy ships to m_quadtree
	for(auto& ship : GameState::ships) {
		m_quadtree->AddObject(ship.second.first);
		
		//ship.second.first->Draw();
		ship.second.first->GetSprite()->RemoveFromDrawing();
		
		ship.second.first->UpdateHullVertices(GameState::asset.GetTextureHull("ship_01_skin_collision.png").vertices);
		if(Command::Get("collisionhull"))
			ship.second.first->RenderCollisionHull();
	}
		
	/*
	//check place_ship
	// add clicked ships to m_quadtree
	for(auto& ship : ships) {
		m_quadtree->AddObject(ship);
		
		ship->UpdateHullVertices(GameState::asset.GetTextureHull("ship_01_skin_collision.png").vertices);
		if(Command::Get("collisionhull"))
			ship->RenderCollisionHull();
	}
	*/
	
	// add my ship to quadtree
	m_quadtree->AddObject(&ship);
	
	// quadtree
	std::vector<Object*> drawObjects;
	m_quadtree->QueryRectangle(ship.GetPosition().x - GameState::windowSize.x/2*GameState::zoom, ship.GetPosition().y - GameState::windowSize.y/2*GameState::zoom, GameState::windowSize.x*GameState::zoom, GameState::windowSize.y*GameState::zoom, drawObjects);
	for(auto& object : drawObjects) {
		//if((SolidObject*)object == (SolidObject*)&ship) continue;
		
		((SolidObject*)object)->Draw();
	}
	
	Collision::Check(m_quadtree);
	
	//needs fix
	/*
	std::vector<Object*> objcts = ship.NodePtr->GetObjectsInNode();
	for(auto& object2 : objcts) {
		m_quadtree->DrawRect(object2->GetPosition().x - object2->GetSize().x/2, object2->GetPosition().y - object2->GetSize().y/2, object2->GetSize().x, object2->GetSize().y, glm::vec4(0, 0, 1, 1));
	}
	*/
	
	if(Command::Get("quadtree")) {
		m_quadtree->Draw();
	}

	if(Command::Get("aabb")) {
		m_quadtree->DrawRect(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, ship.GetSize().x, ship.GetSize().y, glm::vec4(0, 1, 0, 1));
	}
	
	ship.UpdateHullVertices(GameState::asset.GetTextureHull("ship_01_skin_collision.png").vertices);
	if(Command::Get("collisionhull")) {
		ship.RenderCollisionHull();
	}
	
	tb_game_user_name->SetImage(std::string(TEXTURE_PATH) + std::string("hud_1.png"));
	tb_game_user_name->SetText(std::to_string(GameState::user_id) + " | " + GameState::user_name + std::string("\nlel"));
	//tb_game_user_name->SetRect(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, 200, 28);
	
	/*
	for(Projectile& projectile : GameState::projectiles) {
		projectile.Draw();
	}
	*/
	
	//ship.Draw(); //is handled by drawObjects quadtree
	GameState::asset.RenderSprites();
	
	m_quadtree->Clear();
	
	// debug
	showFPS();
	std::string debugString(
		/*
		"App:m_windowSize: " + std::to_string(this->getWindowSize().x) + "x" + std::to_string(this->getWindowSize().y)  + "\n" +
		"App:m_screenMousePosition: " + std::to_string(this->getScreenMousePosition().x) + "," + std::to_string(this->getScreenMousePosition().y)  + "\n" +
		*/
		
		"App:m_worldMousePosition: " + std::to_string(this->getWorldMousePosition().x) + "," + std::to_string(this->getWorldMousePosition().y)  + "\n" +
		"Ship:m_position (center): " + std::to_string(ship.GetPosition().x) + "," + std::to_string(ship.GetPosition().y)  + "\n" +
		"Ship::m_speed: " + std::to_string(glm::length(ship.GetSpeed()))  + "\n" +
		"GameState::objectsDrawn: " + std::to_string(GameState::objectsDrawn)  + "\n" +
		// "Quadtree::DrawnOnScreen: " + std::to_string((drawObjects.size())) + "\n" +
		// "Quadtree::GetObjects: " + std::to_string((nearObjects.size())) + "\n" +
		"projectiles fired: " + std::to_string(GameState::projectiles.size()) + "\n" +
		"FPS: " + std::to_string(m_frames_current) + "\n" +
		GameState::debug_string
	);
	for(auto& d : GameState::debug_fields) {
		debugString += d.first + ": " + d.second + "\n";
	}
	tb_debug->SetText(debugString);
	
	
	// cleanup dead projectiles
	for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
		if(it->IsDead()) {
			it = GameState::projectiles.erase(it);
			if(it == GameState::projectiles.end()) break;
		}
	}
}

void App::loop() {
    // fps
    m_frames_elapsed++;
    m_ticks_current = SDL_GetTicks();

    // delta time
    m_delta_time = (m_ticks_current - m_ticks_then) / 1000.0f;
    m_ticks_then = m_ticks_current;

    // time elapsed
    auto m_chrono_now = std::chrono::high_resolution_clock::now();
    m_chrono_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(m_chrono_now - m_chrono_start).count();
}

void App::showFPS() {
    if(m_ticks_previous < m_ticks_current - 1000) {
        m_ticks_previous = SDL_GetTicks();
        m_frames_current = m_frames_elapsed;
        m_frames_elapsed = 0;

        if(m_frames_current < 1) {
            m_frames_current = 1;
        }

        // std::cout << "FPS: " << m_frames_current << std::endl;
    }
}

void App::setWindowSize(glm::vec2 windowSize) {
    m_windowSize = windowSize;
}

void App::setScreenMousePosition(glm::vec2 screenMousePosition) {
    m_screenMousePosition = screenMousePosition;
}

void App::setWorldMousePosition(glm::vec2 worldMousePosition) {
    m_worldMousePosition = worldMousePosition;
}

void App::setZoom(float zoom) {
	m_zooming = zoom;
}

glm::vec2 App::getWindowSize() const {
    return m_windowSize;
}

glm::vec2 App::getScreenMousePosition() const {
    return m_screenMousePosition;
}

glm::vec2 App::getWorldMousePosition() const {
    return m_worldMousePosition;
}

double App::getDeltaTime() const {
    return m_delta_time;
}

double App::getTimeElapsed() const {
    return m_chrono_elapsed;
}

float App::getZoom() const {
	return m_zooming;
}

// TODO:
bool App::server_doPassRestore(std::string user_email) {
	//if email was not restored >10 mins ago, return true; else false;
	
	return true;
}

void App::generate_RSA_keypair() {
	CryptoPP::AutoSeededRandomPool rng;

	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, RSA_KEY_SIZE);

	CryptoPP::RSA::PublicKey publicKey(params);
	CryptoPP::RSA::PrivateKey privateKey(params);

	publicKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(GameState::clientPublicKeyStr)).Ref());
	privateKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(GameState::clientPrivateKeyStr)).Ref());
}

void App::init_audio() {
	if(!Mix_Init(MIX_INIT_OGG)) {
		throw std::string("Failed to initialize SDL_mixer");
	}
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
		/*
		 * ALSA lib pcm_dmix.c:1079:(snd_pcm_dmix_open) unable to open slave
		 * Mix_OpenAudio: ALSA: Couldn't open audio device: File descriptor in bad state
		 * Even tho ALSA lib complains, Mix_OpenAudio throws an error, but if its not thrown, music continue to play and everything works correctly.
		 * Commented during development
		*/
		//throw std::string("Mix_OpenAudio: ") + Mix_GetError();
	}
	Mix_PlayMusic(GameState::asset.GetMusic("loop.ogg"), -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME);
}

void App::init_commands() {
	ng::Terminal &tm_game_chat = *((ng::Terminal*)GameState::gui.GetControlById("game_terminal"));
	
	// -----------------[ commands ]-----------------------
	Command::AddCommand("w", [&](std::string nick, std::string message) {
		tm_game_chat.WriteLog("^p[pm to " + nick + "]^w: " + message + "^w");
		NetworkChat::SendChatMessage(nick, message);
	});
	
	Command::AddCommand("fullscreen", [&]() {
		toggleFullscreen = !toggleFullscreen;
				
		if(toggleFullscreen) {
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

			int w, h;
			SDL_GetWindowSize(window, &w, &h);
			this->setWindowSize(glm::vec2(w, h));

			skipMouseResolution = 4;
		} else {
			SDL_SetWindowFullscreen(window, 0);
			this->setWindowSize(m_initialWindowSize);

			skipMouseResolution = 4;
		}
	});
	
	Command::AddCommand("wireframe", [&]() {
		toggleWireframe = !toggleWireframe;
				
		if(toggleWireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	});
	
	Command::AddCommand("quit", [&]() {
		m_running = false;
	});
	
	Command::AddCommand("clear", [&]() {
		tm_game_chat.ClearLog();
	});
	
	Command::AddCommand("talk", [&]() {
		tm_game_chat.Focus();
	});
	
	Command::AddCommand("say", [&](std::string msg) {
		tm_game_chat.WriteLog(GameState::user_name + ": " + msg + "^w");
		NetworkChat::SendChatMessage("", msg);
		cout << "say: " << msg << endl;
	});
}

COMMAND(void, set_swap_interval, (int interval)) {
	SDL_GL_SetSwapInterval(interval);
}

COMMAND(void, setspeed, (float speed)) {
	cout << "speed: " << speed << endl;
}

void App::on_exit() {
	m_bind.SaveKeys("galaxy31.cfg");
	Command::SaveVarariablesToFile("galaxy31.cfg",false);
	Network::SendGoodBye();
}
