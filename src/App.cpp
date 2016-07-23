#include "App.hpp"
#include "Network.hpp"
App::App() {
    this->setWindowSize(glm::vec2(800, 600));

    m_ticks_previous = SDL_GetTicks();
    m_ticks_current = 0;
    m_frames_current = 0;
    m_frames_elapsed = 0;

    m_delta_time = 0;
    m_ticks_then = 0;

    m_chrono_start = std::chrono::high_resolution_clock::now();
    m_chrono_elapsed = 0;

    this->init();
}

App::~App() {
	GameState::asset.FreeAssets();
    Mix_CloseAudio();
    Mix_Quit();
    Network::cleanup();
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void App::init() {
    Network::initialize();
    Network::connect("127.0.0.1", 1234);
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
		throw std::string("Mix_OpenAudio: ") + Mix_GetError();
	}
	Mix_PlayMusic(GameState::asset.GetMusic("loop.ogg"), -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME);

    glContext = SDL_GL_CreateContext(window);
    if(glContext == nullptr) {
        throw std::string("Failed to create GLContext: ") + SDL_GetError();
    }

    SDL_GL_SetSwapInterval(0);
    SDL_GL_MakeCurrent(window, glContext);

    glewExperimental = GL_TRUE;
    glewInit();
    //gl3wInit();

    ImGui_ImplSdlGL3_Init(window);

    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    bool toggleMouseRelative = false;
    bool toggleFullscreen = true;
    bool toggleWireframe = true;

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetWindowGrab(window, SDL_TRUE);

    if(toggleMouseRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    
    //load all textures here
    GameState::asset.LoadTexture("ship_01_skin.png");
    GameState::asset.LoadTexture("propulsion.png");
    
    //std::cout << GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png") << std::endl;
    //std::cout << GameState::asset.GetTexture("Assets/Textures/propulsion.png") << std::endl;
    
    GameState::asset.LoadShader("background.vs", "background.fs");
    GameState::asset.LoadShader("sprite.vs", "sprite.fs");
    
    //std::cout << GameState::asset.GetShader("Assets/Shaders/background.vs") << std::endl;
    //std::cout << GameState::asset.GetShader("Assets/Shaders/sprite.vs") << std::endl;

	Ship::Chassis chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
    Ship ship(glm::vec2(0, 0), 0.0, chassis);
    Ship ship2(glm::vec2(0, 0), 0.0, chassis);
    
    int skipMouseResolution = 0;

    bool running = true;
	bool isFiring = false;
    while(running) {
        this->loop();

        SDL_Event e;

        while(SDL_PollEvent(&e)) {
            ImGui_ImplSdlGL3_ProcessEvent(&e);
            if(e.type == SDL_QUIT) {
                running = false;
            } else if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
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

                        /*
                        int w, h;
                        SDL_GetWindowSize(window, &w, &h); //?
                        */

                        this->setWindowSize(glm::vec2(800, 600));

                        toggleFullscreen = true;
                    }
                    
                    skipMouseResolution = 4;
                }
                break;

                case SDLK_ESCAPE:
                    running = false;
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
                }
            } else if(e.type == SDL_MOUSEBUTTONDOWN) {
				if(e.button.button == SDL_BUTTON_LEFT) {
					isFiring = true;
				} else {
					ship.Stabilizers();
				}
            } else if(e.type == SDL_MOUSEBUTTONUP) {
				if(e.button.button == SDL_BUTTON_LEFT) {
					isFiring = false;
				} else {
					ship.Stabilizers();
				}
            } else if(e.type == SDL_MOUSEWHEEL) {
                this->setZoom(std::max(this->getZoom() - e.wheel.y, 1.0f));
            }
        }
        
		if(isFiring) ship.Fire();
        ImGui_ImplSdlGL3_NewFrame(window);

        ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImColor(1.0f, 1.0f, 1.0f, 0.5f);
        ImGui::GetStyle().WindowRounding = 2.0f;
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImColor(1.0f, 1.0f, 1.0f, 0.25f);

        //ImGui::SetNextWindowPos(ImVec2(0,0));
        //ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        //ImGui::Begin("text", NULL, ImVec2(0,0), 0.0f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
        {
            //static float f = 0.0f;
            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("App:m_windowSize: %.0fx%.0f", this->getWindowSize().x, this->getWindowSize().y);
            ImGui::Text("App:m_screenMousePosition: %.0f,%.0f", this->getScreenMousePosition().x, this->getScreenMousePosition().y);
            ImGui::Text("App:m_worldMousePosition: %.0f,%.0f", this->getWorldMousePosition().x, this->getWorldMousePosition().y);
            ImGui::Text("Ship:m_position (center): %.0f,%.0f", ship.GetPosition().x, ship.GetPosition().y);
            ImGui::Text("Ship::m_speed: %.2f", glm::length(ship.GetSpeed()));
        }
        //ImGui::End();
        
        {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			static ImVector<ImVec2> points;

			ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
			/*
			if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
			if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
			*/
			//draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(50,50,50), ImColor(50,50,60), ImColor(60,60,70), ImColor(50,50,60));
			draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255,255,255));
			
			int pointX = (ship.GetPosition().x / (2 * GameState::worldSize.x) * canvas_size.x) + canvas_size.x/2.0f;
			int pointY = (ship.GetPosition().y / (2 * GameState::worldSize.y) * canvas_size.y) + canvas_size.y/2.0f;
			draw_list->AddCircleFilled(ImVec2(canvas_pos.x + pointX, canvas_pos.y + pointY), 2.0f, 0xFF00FFFF, 12);
			
			int point2X = (ship2.GetPosition().x / (2 * GameState::worldSize.x) * canvas_size.x) + canvas_size.x/2.0f;
			int point2Y = (ship2.GetPosition().y / (2 * GameState::worldSize.y) * canvas_size.y) + canvas_size.y/2.0f;
			draw_list->AddCircleFilled(ImVec2(canvas_pos.x + point2X, canvas_pos.y + point2Y), 2.0f, 0xFF0000FF, 12);
			
			draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y));      // clip lines within the canvas (if we resize it, etc.)
			draw_list->PopClipRect();
		}

        glViewport(0, 0, this->getWindowSize().x, this->getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);

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
        
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("background.vs").id, "iMouse"), ship.GetPosition().x, -ship.GetPosition().y);
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("background.vs").id, "iResolution"), this->getWindowSize().x, this->getWindowSize().y);

        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);

        glUseProgram(0);
        //
        
        //some rounding error? (still moving ship outside of the world, check some corner)
        if(std::abs(ship.GetPosition().x) >= GameState::worldSize.x && std::abs(this->getWorldMousePosition().x) >= std::abs(ship.GetPosition().x)) {
			ship.SetSpeed(glm::vec2(0, ship.GetSpeed().y));
		}
		
		if(std::abs(ship.GetPosition().y) >= GameState::worldSize.y && std::abs(this->getWorldMousePosition().y) >= std::abs(ship.GetPosition().y)) {
			ship.SetSpeed(glm::vec2(ship.GetSpeed().x, 0));
		}
		Network::send_message("hehehe");
		Network::handle_events(5);
		ship.Process();
		for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
			it->Process();
			if(it->IsDead()) {
				it = GameState::projectiles.erase(it);
				if(it == GameState::projectiles.end()) break;
			}
		}
        //ship2.Process();

        GameState::camera.SetPosition( glm::vec3(ship.GetPosition().x, ship.GetPosition().y, 0) );
        glm::mat4 projection = glm::ortho(-(float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().y*this->getZoom()*0.5, -(float)this->getWindowSize().y*this->getZoom()*0.5);
        glm::mat4 view = GameState::camera.GetViewMatrix();
        GameState::camera.SetProjection(projection);
        GameState::camera.SetView(view);
        
        ship.Draw();
        ship2.Draw();
        
        for(Projectile& projectile : GameState::projectiles) {
			projectile.Draw();
		}
        
        ImGui::Render();

        SDL_GL_SwapWindow(window);

        this->showFPS();

        //SDL_Delay(16);
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

        std::cout << "FPS: " << m_frames_current << std::endl;
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
