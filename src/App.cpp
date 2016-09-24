#include "App.hpp"
#include "Network.hpp"
#include "GameState.hpp"
#include "Quadtree.hpp"
#include "commands/commands.hpp"
#include "commands/bind.hpp"
#include "server/network.hpp"

#include <sstream>

using Commands::Arg;

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
    
    this->init();
}

COMMAND(std::string, concat, (std::vector<Arg> args)) {
	// cout << "func: " << a << ", " << b << endl;
	std::string str = "";
	for(auto& a : args) {
		if(a.type == Arg::t_int) 
			str = str + std::to_string(a.i);
		if(a.type == Arg::t_string) {
			str += a.s;
		} 
	}
	return str;
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

void App::init() {
	
	
    Network::initialize();
    NetworkChat::initialize();
    // Network::connect("89.177.76.215", SERVER_PORT);
    Network::connect("127.0.0.1", SERVER_PORT);
    
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
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    
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

    bool toggleMouseRelative = false;
    bool toggleFullscreen = false;
    bool toggleWireframe = false;
    
    int skipMouseResolution = 0;
	bool wait_for_packets = false;
    bool running = true;
	bool isFiring = false;

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetWindowGrab(window, SDL_TRUE);

    if(toggleMouseRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    
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
	//GameState::gui->SetSize(this->getWindowSize().x, this->getWindowSize().y);
	//Drawing::SetResolution( this->getWindowSize().x, this->getWindowSize().y );
	//Drawing::Init();
	
	GameState::gui.SetDefaultFont("Assets/Fonts/DroidSansMono.ttf");
	GameState::gui.LoadXml("Assets/gui.xml");
	GameState::gui.ApplyAnchoring();
	
	TextBox* tb_debug = (TextBox*)GameState::gui.GetControlById("game_debug");
	//tb_debug->SetRect(0, 0, 100, 50);
	//tb_debug->SetBackgroundColor(0);
	
	Canvas* cv_minimap = (Canvas*)GameState::gui.GetControlById("game_minimap");
	//cv_minimap->SetReadOnly(true);
	//cv_minimap->SetBackgroundColor(0);
	
	GameState::activePage = "login";

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
		
		if(this->TODOserver_doPassRestore(tb_pass_restore_email->GetText())) {
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

	Button &bt_login_register = *((Button*)GameState::gui.GetControlById("login_register"));
	bt_login_register.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::activePage = "register";
	});
	
	Button &bt_login_pass_restore = *((Button*)GameState::gui.GetControlById("login_pass_restore"));
	bt_login_pass_restore.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::activePage = "pass_restore";
	});
	
	Button &bt_register_login = *((Button*)GameState::gui.GetControlById("register_login"));
	bt_register_login.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::activePage = "login";
	});
	
	Button &bt_pass_restore_login = *((Button*)GameState::gui.GetControlById("pass_restore_login"));
	bt_pass_restore_login.SubscribeEvent(Button::event::click, [&](Control* c) {
		GameState::activePage = "login";
	});
	
	/*
	Terminal &tm_game_chat = *((Terminal*)GameState::gui.GetControlById("game_terminal"));
	tm_game_chat.SubscribeEvent(Terminal::event::command, [&](Control* c) {
		Terminal* t = (Terminal*)c;
		
		std::vector<std::string> exploded;
		
		std::string buf;
		std::stringstream ss(t->GetText());

		int i = 0;
		while(ss >> buf && i < 3) {
			exploded.push_back(buf);
			i++;
		}
		
		if(exploded.size() != 0 && exploded[0] == "/w") {
			if(GameState::user_name != exploded[1]) {
				t->WriteLog("^p[pm to " + exploded[1] + "]^w: " + &t->GetText()[exploded[0].size()+exploded[1].size()+2] + "^w");
				NetworkChat::SendChatMessage(exploded[1], &t->GetText()[exploded[0].size()+exploded[1].size()+2]);
			}
		} else {
			t->WriteLog(GameState::user_name + ": " + t->GetText() + "^w");
			NetworkChat::SendChatMessage("", t->GetText());
		}
	});
	*/
	
	Terminal &tm_game_chat = *((Terminal*)GameState::gui.GetControlById("game_terminal"));
	
	// commands
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
		running = false;
	});
	
	Command::AddCommand("clear", [&]() {
		tm_game_chat.ClearLog();
	});
	
	Command::AddCommand("talk", [&]() {
		tm_game_chat.Focus();
	});
	
	Command::AddCommand("say", [&](std::string msg) {
		tm_game_chat.WriteLog("\n" + GameState::user_name + ": " + msg + "^w");
		NetworkChat::SendChatMessage("", msg);
		cout << "say: " << msg << endl;
	});
	
	
	
	Command::AddCommand("loop", [&](int n, Arg e) {
		std::vector<Arg> arg;
		for(int i=0; i < n; i++)
			Command::Execute(e, arg);
	});
	
	tm_game_chat.SubscribeEvent(Terminal::event::command, [&](Control* c) {
		Terminal* t = (Terminal*)c;
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
		
	Ship::Chassis chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
    Ship ship(glm::vec2(0, 0), 0.0, chassis);
    GameState::player = &ship;
    
    Quadtree* quadtree = new Quadtree ( -GameState::worldSize.x, GameState::worldSize.x, -GameState::worldSize.y, GameState::worldSize.x, 6 ) ;

    std::vector<Ship*> ships;
    /*
    for(int x = 0; x < 20; ++x) {
		for(int y = 0; y < 20; y++) {
			Ship* ship = new Ship({x*1000, y*1000}, 0.0, chassis);
			ships.push_back(ship);
		}
	}
	*/
	Command::LoadFromFile("galaxy31.cfg");
    while(running) {
        this->loop();

        SDL_Event e;

		GameState::input_taken = GameState::gui.GetActiveControl() != nullptr;
        while(SDL_PollEvent(&e)) {
			

            if(e.type == SDL_QUIT) {
                running = false;
            } else if(e.type == SDL_KEYDOWN) {
				if(!GameState::input_taken) {
					if(m_bind.OnEvent(e))
						continue;
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
						break;
						case SDLK_SLASH:
							tm_game_chat.Focus();
							break;
						
						/*
						case SDLK_f: {
							if(toggleFullscreen) {
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

								int w, h;
								SDL_GetWindowSize(window, &w, &h);
								this->setWindowSize(glm::vec2(w, h));

								toggleFullscreen = false;
							} else {
								SDL_SetWindowFullscreen(window, 0);
								//SDL_SetWindowDisplayMode(window, 0);

								
								//int w, h;
								//SDL_GetWindowSize(window, &w, &h); //?

								this->setWindowSize(m_initialWindowSize);

								toggleFullscreen = true;
							}
							
							skipMouseResolution = 4;
						}
						break;
						
						case SDLK_e: {
							if(toggleWireframe) {
								glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
								toggleWireframe = false;
							} else {
								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
								toggleWireframe = true;
							}
						}
						break;
						*/
					}
				}
            } else if(e.type == SDL_MOUSEBUTTONDOWN && GameState::gui.GetSelectedControl() == nullptr) {
				
				if(e.button.button == SDL_BUTTON_LEFT) {
					Ship* ship = new Ship(this->getWorldMousePosition(), 0.0, chassis);
					ships.push_back(ship);
					
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

            this->setWorldMousePosition(glm::vec2(
                                            ((float)this->getScreenMousePosition().x - this->getWindowSize().x * 0.5) * m_zooming + GameState::camera.GetPosition().x,
                                            ((float)this->getScreenMousePosition().y - this->getWindowSize().y * 0.5) * m_zooming + GameState::camera.GetPosition().y
                                        ));
        }
        
        GameState::windowSize = this->getWindowSize();
        GameState::screenMousePosition = this->getScreenMousePosition();
        GameState::worldMousePosition = this->getWorldMousePosition();
        GameState::deltaTime = this->getDeltaTime();
        GameState::timeElapsed = this->getTimeElapsed();
        GameState::zoom = this->getZoom();
        GameState::objectsDrawn = 0;
        
        Network::handle_events(5);
        
        if(GameState::activePage == "login") {
			GameState::gui.GetControlById("login")->SetVisible(true);
			GameState::gui.GetControlById("register")->SetVisible(false);
			GameState::gui.GetControlById("pass_restore")->SetVisible(false);
			//GameState::gui.GetControlById("lobby")->SetVisible(false);
			GameState::gui.GetControlById("game")->SetVisible(false);
		} else if(GameState::activePage == "register") {
			GameState::gui.GetControlById("login")->SetVisible(false);
			GameState::gui.GetControlById("register")->SetVisible(true);
			GameState::gui.GetControlById("pass_restore")->SetVisible(false);
			//GameState::gui.GetControlById("lobby")->SetVisible(false);
			GameState::gui.GetControlById("game")->SetVisible(false);
		} else if(GameState::activePage == "pass_restore") {
			GameState::gui.GetControlById("login")->SetVisible(false);
			GameState::gui.GetControlById("register")->SetVisible(false);
			GameState::gui.GetControlById("pass_restore")->SetVisible(true);
			//GameState::gui.GetControlById("lobby")->SetVisible(false);
			GameState::gui.GetControlById("game")->SetVisible(false);
		} else if(GameState::activePage == "game") {
			GameState::gui.GetControlById("login")->SetVisible(false);
			GameState::gui.GetControlById("register")->SetVisible(false);
			GameState::gui.GetControlById("pass_restore")->SetVisible(false);
			//GameState::gui.GetControlById("lobby")->SetVisible(false);
			GameState::gui.GetControlById("game")->SetVisible(true);
		}

		// background
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
			NetworkChat::handle_events(5);
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
			//
			
			//Network::handle_events(5);
			ship.Process();
			for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
				it->Process();
			}
			
			// handle multiplayer states interpolation (this code should be moved elsewhere later)
			for(auto& obj : GameState::ships) {
				auto& p = obj.second;
				// if(wait_for_packets) {
					// if(p.second.size() > 5)
						// wait_for_packets = false;
					// break;
				// }
				// else if(p.second.size() < 2)
					// wait_for_packets = true;
				
				while(p.second.size() > 1 && p.second.front().GetTicks() <= p.first->GetTicks()) {
					// std::cout << "poping : " << p.second.size() << std::endl;
					p.second.pop();
				}
				
				if(!p.second.empty() && p.second.front().GetTicks() <= p.first->GetTicks()) {
					// std::cout << "copy state\n";
					p.first->CopyObjectState(p.second.front());
					p.second.pop();
				}
				
				double dtime = GameState::deltaTime*1000000.0;
				if(p.second.empty()) {
					// cout << "processing\n";
					((Object*)p.first)->Process();
				} else {
					double diff = p.second.front().GetTicks()+dtime - p.first->GetTicks();
					// std::cout << "interpolating [" << p.second.size() << "] : " << (dtime) << "  (" << p.second.front().GetTicks() << ", " << p.first->GetTicks() << ") " << "diff: " << diff << "\n";
					if(diff > dtime)
						p.first->InterpolateToState(p.second.front(), dtime / diff );
				}
				p.first->AddTicks( dtime );
				
			}
			
			if(isFiring) ship.Fire();

			GameState::camera.SetPosition( glm::vec3(ship.GetPosition().x, ship.GetPosition().y, 0) );
			glm::mat4 projection = glm::ortho(-(float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().y*this->getZoom()*0.5, -(float)this->getWindowSize().y*this->getZoom()*0.5);
			glm::mat4 view = GameState::camera.GetViewMatrix();
			GameState::camera.SetProjection(projection);
			GameState::camera.SetView(view);
			
			Network::SendOurState();
			
			for(auto& projectile : GameState::projectiles) {
				quadtree->AddObject(&projectile);
				
				projectile.UpdateHullVertices(GameState::asset.GetTextureHull("projectile_collision.png").vertices);
				if(Command::Get("collisionhull"))
				projectile.RenderCollisionHull();
			}
			
			for(auto& obj : GameState::ships) {
				quadtree->AddObject(obj.second.first);
			}
			
			for(auto& obj : GameState::ships) {
				obj.second.first->CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
			
				std::unordered_map<Object*, Quadtree*> nearObjects;
				quadtree->QueryRectangle(obj.second.first->GetPosition().x - obj.second.first->GetSize().x/2, obj.second.first->GetPosition().y - obj.second.first->GetSize().y/2, obj.second.first->GetSize().x, obj.second.first->GetSize().y, nearObjects);
				for(auto& object : nearObjects) {
					if((SolidObject*)obj.second.first != (SolidObject*)object.first) {
						//quadtree.DrawRect(object.first->GetPosition().x - object.first->GetSize().x/2, object.first->GetPosition().y - object.first->GetSize().y/2, object.first->GetSize().x, object.first->GetSize().y, glm::vec4(1, 1, 1, 1));
						
						if(obj.second.first->Collides((SolidObject*)object.first)) {
							std::cout << "collides!" << std::endl;
							obj.second.first->CollisionHullColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
						}
					}
				}
			}
			
			for(auto& obj : GameState::ships) {
				obj.second.first->Draw();
				
				obj.second.first->UpdateHullVertices(GameState::asset.GetTextureHull("ship_01_skin_collision.png").vertices);
				if(Command::Get("collisionhull"))
				obj.second.first->RenderCollisionHull();
			}
			
			for(auto& ship : ships) {
				quadtree->AddObject(ship);
			}
			
			// quadtree
			std::unordered_map<Object*, Quadtree*> drawObjects;
			quadtree->QueryRectangle(ship.GetPosition().x - GameState::windowSize.x/2*GameState::zoom, ship.GetPosition().y - GameState::windowSize.y/2*GameState::zoom, GameState::windowSize.x*GameState::zoom, GameState::windowSize.y*GameState::zoom, drawObjects);
			for(auto& object : drawObjects) {
				((SolidObject*)object.first)->Draw();
			}
			
			ship.CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
			
			std::unordered_map<Object*, Quadtree*> nearObjects;
			quadtree->QueryRectangle(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, ship.GetSize().x, ship.GetSize().y, nearObjects);
			for(auto& object : nearObjects) {
				if((SolidObject*)&ship != (SolidObject*)object.first) {
					//quadtree->DrawRect(object.first->GetPosition().x - object.first->GetSize().x/2, object.first->GetPosition().y - object.first->GetSize().y/2, object.first->GetSize().x, object.first->GetSize().y, glm::vec4(1, 1, 1, 1));
					
					if(ship.Collides((SolidObject*)object.first)) {
						std::cout << "collides!" << std::endl;
						ship.CollisionHullColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
					}
				}
				
				if(object.second == nullptr) continue;
				
				std::vector<Object*> objcts = object.second->GetObjectsInNode();
				for(auto& object2 : objcts) {
					quadtree->DrawRect(object2->GetPosition().x - object2->GetSize().x/2, object2->GetPosition().y - object2->GetSize().y/2, object2->GetSize().x, object2->GetSize().y, glm::vec4(0, 0, 1, 1));
				}
			}
			
			if(Command::Get("quadtree")) {
				quadtree->Draw();
			}
			quadtree->Clear();
			if(Command::Get("aabb"))
			quadtree->DrawRect(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, ship.GetSize().x, ship.GetSize().y, glm::vec4(0, 1, 0, 1));
			//
			
			ship.Draw();
			ship.UpdateHullVertices(GameState::asset.GetTextureHull("ship_01_skin_collision.png").vertices);
			
			if(Command::Get("collisionhull"))
			ship.RenderCollisionHull();
			
			TextBox* tb_game_user_name = (TextBox*)GameState::gui.GetControlById("game_user_name");
			tb_game_user_name->SetText(std::to_string(GameState::user_id) + " | " + GameState::user_name);
			//tb_game_user_name->SetRect(ship.GetPosition().x - ship.GetSize().x/2, ship.GetPosition().y - ship.GetSize().y/2, 200, 28);
			
			for(Projectile& projectile : GameState::projectiles) {
				projectile.Draw();
			}
			
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
				"Quadtree::DrawnOnScreen: " + std::to_string((drawObjects.size())) + "\n" +
				"Quadtree::GetObjects: " + std::to_string((nearObjects.size())) + "\n" +
				"FPS: " + std::to_string(m_frames_current)
			);
			tb_debug->SetText(debugString);
			
			for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
				// it->Process();
				if(it->IsDead()) {
					it = GameState::projectiles.erase(it);
					if(it == GameState::projectiles.end()) break;
				}
			}
		}
	
        
        GameState::gui.Render();

        SDL_GL_SwapWindow(window);

        // this->showFPS();
        

        // SDL_Delay(16);
    }
    
    m_bind.SaveKeys("galaxy31.cfg");
	Command::SaveVarariablesToFile("galaxy31.cfg");
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

bool App::TODOserver_doPassRestore(std::string user_email) {
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


COMMAND(void, setspeed, (float speed)) {
	cout << "speed: " << speed << endl;
}
