#include "App.hpp"

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

App::~App() {}

void App::init() {
    SDL_Window* window;
    SDL_GLContext glContext;

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
    GameState::asset.LoadTexture("Assets/Textures/ship_01_skin.png");
    GameState::asset.LoadTexture("Assets/Textures/propulsion.png");
    
    //std::cout << GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png") << std::endl;
    //std::cout << GameState::asset.GetTexture("Assets/Textures/propulsion.png") << std::endl;
    
    GameState::asset.LoadShader("Assets/Shaders/background.vs", "Assets/Shaders/background.fs");
    GameState::asset.LoadShader("Assets/Shaders/sprite.vs", "Assets/Shaders/sprite.fs");
    
    //std::cout << GameState::asset.GetShader("Assets/Shaders/background.vs") << std::endl;
    //std::cout << GameState::asset.GetShader("Assets/Shaders/sprite.vs") << std::endl;

    Ship ship(glm::vec2(0, 0), 0.0, glm::vec2(0, 0), 0.001, 0, "some name", GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png"), GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png"), 100.0, 100.0);
    Ship ship2(glm::vec2(0, 0), 0.0, glm::vec2(0, 0), 0.001, 0, "some name", GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png"), GameState::asset.GetTexture("Assets/Textures/ship_01_skin.png"), 100.0, 100.0); 
    
    int skipMouseResolution = 0;

    bool running = true;

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
            } else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {

            } else if(e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {

            } else if(e.type == SDL_MOUSEWHEEL) {
                this->setZoom(std::max(this->getZoom() - e.wheel.y, 1.0f));
            }
        }

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
        glUseProgram(GameState::asset.GetShader("Assets/Shaders/background.vs").id);

        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/background.vs").id, "iMouse"), ship.GetPosition().x, -ship.GetPosition().y);
        glUniform2f(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/background.vs").id, "iResolution"), this->getWindowSize().x, this->getWindowSize().y);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDeleteVertexArrays(1, &VAO);

        glUseProgram(0);
        //

        ship.Process();
        //ship2.Process();

        GameState::camera.SetPosition( glm::vec3(ship.GetPosition().x, ship.GetPosition().y, 0) );
        glm::mat4 projection = glm::ortho(-(float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().x*this->getZoom()*0.5, (float)this->getWindowSize().y*this->getZoom()*0.5, -(float)this->getWindowSize().y*this->getZoom()*0.5);
        glm::mat4 view = GameState::camera.GetViewMatrix();
        GameState::camera.SetProjection(projection);
        GameState::camera.SetView(view);
        
        ship.Draw();
        ship2.Draw();
        
        ImGui::Render();

        SDL_GL_SwapWindow(window);

        this->showFPS();

        //SDL_Delay(16);
    }

    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
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
