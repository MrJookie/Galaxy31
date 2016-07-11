#include "App.hpp"

App::App()
{
    m_sizeX = 800;
    m_sizeY = 600;
    
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

void App::init()
{
	SDL_Window* window;
    SDL_GLContext glContext;
 
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::string("Failed to initialize SDL: ") + SDL_GetError();
    }

    window = SDL_CreateWindow("Galaxy31", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_sizeX, m_sizeY, SDL_WINDOW_OPENGL);
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
    
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_SetWindowGrab(window, SDL_TRUE);
	
    bool toggleMouseRelative = false;
    bool toggleFullscreen = true;
    bool toggleWireframe = true;
    
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    //SDL_SetWindowGrab(window, SDL_TRUE);
    
    if(toggleMouseRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    
	Shader backgroundShader("Assets/background.vs", "Assets/background.fs");
    Shader spriteShader("Assets/sprite.vs", "Assets/sprite.fs");
    Ship ship("Assets/SpaceShip01.png");
    ship.SetSpriteShader(spriteShader.GetShader());
    ship.SetPosition(100,100);
    
    float speed = 100.0f;
    
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

    int skipMouseResolution = 0;
    
    bool running = true;
    
    while(running)
    {
		this->loop();
		
        SDL_Event e;

        while(SDL_PollEvent(&e))
        {
			ImGui_ImplSdlGL3_ProcessEvent(&e);
            if(e.type == SDL_QUIT) {
                running = false;
            }
            else if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym)
                {
					case SDLK_f:
                    {       
                        if(toggleFullscreen) {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            
                            int w, h;
                            SDL_GetWindowSize(window, &w, &h);
                            m_sizeX = w;
                            m_sizeY = h;
                            
                            toggleFullscreen = false;
                        }
                        else {
                            SDL_SetWindowFullscreen(window, 0);
                            //SDL_SetWindowDisplayMode(window, 0);

                            int w, h;
                            SDL_GetWindowSize(window, &w, &h); //?
                            
                            m_sizeX = 800;
                            m_sizeY = 600;
                            
                            toggleFullscreen = true;
                        }
                        
                        skipMouseResolution = 2;     
                    }
                    break;
                    
					case SDLK_ESCAPE:
						running = false;
					break;
					
					case SDLK_e:
                    {
                        if(toggleWireframe) {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                            toggleWireframe = false;
                        }
                        else {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                            toggleWireframe = true;
                        }
                    }
                    break;
                }
            }
            else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
                toggleMouseRelative = true;

                skipMouseResolution = 2;
            }
            else if(e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
                toggleMouseRelative = false;

                skipMouseResolution = 2;
            }
            else if(e.type == SDL_MOUSEWHEEL) {
				m_zooming = std::max(m_zooming - e.wheel.y, 1.0f);
				
				
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
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("speed: %.2f", glm::length(ship.GetSpeed()));
        }
        //ImGui::End();

        glViewport(0, 0, m_sizeX, m_sizeY);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        
        if(state[SDL_SCANCODE_W]) {
            //camera.ProcessKeyboard(cx::Camera::MoveDirection::FORWARD, getDeltaTime() * 2.0);
        }
        
        if(state[SDL_SCANCODE_S]) {
            //camera.ProcessKeyboard(cx::Camera::MoveDirection::BACKWARD, getDeltaTime() * 2.0);
        }
        
        if(state[SDL_SCANCODE_A]) {
            //camera.ProcessKeyboard(cx::Camera::MoveDirection::LEFT, getDeltaTime() * 2.0);
        }

        if(state[SDL_SCANCODE_D]) {
            //camera.ProcessKeyboard(cx::Camera::MoveDirection::RIGHT, getDeltaTime() * 2.0);
        }
        
        
        
        int xpos;
        int ypos;
        SDL_GetRelativeMouseState(&xpos, &ypos);
        
        if(skipMouseResolution > 0 && (xpos != 0 || ypos != 0)) {
            skipMouseResolution--;
        }
        else {
            if(toggleMouseRelative) {
				//camera.ProcessMouseMovement(xpos, ypos);
            }
            else {
                //int xpos;
                //int ypos;
                SDL_GetMouseState(&xpos, &ypos);
                xpos = ((float)xpos-getSizeX()*0.5)*m_zooming + camera.GetPosition().x;
                ypos = ((float)ypos-getSizeY()*0.5)*m_zooming + camera.GetPosition().y;
                
            }
        }
        
        
        // background
        glUseProgram(backgroundShader.GetShader());
        
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
        glUniform2f(glGetUniformLocation(backgroundShader.GetShader(), "iMouse"), ship.GetPosition().x, -ship.GetPosition().y);
        glUniform2f(glGetUniformLocation(backgroundShader.GetShader(), "iResolution"), this->getSizeX(), this->getSizeY());
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDeleteVertexArrays(1, &VAO);
        
        glUseProgram(0);
        //

        glm::vec2 pos_to_mouse_vector = glm::vec2(
												(float)xpos - ship.GetPosition().x, 
												(float)ypos - ship.GetPosition().y 
											);
		glm::vec2 direction = glm::normalize(pos_to_mouse_vector);
		float distance = glm::length(pos_to_mouse_vector);
        float angle = glm::degrees( glm::orientedAngle( glm::vec2(1.0f,0.0f), direction ) );
        ship.SetRotation( angle + 90 );
        float acceleration = 1.0;
		
		if(distance > ship.GetSize().y/2 + 0.0) {
			ship.Accelerate(direction * distance*0.001f);
		}
		
		ship.Process(this->getDeltaTime());
		
		camera.SetPosition( glm::vec3(ship.GetPosition().x, ship.GetPosition().y, 0) );
        glm::mat4 projection = glm::ortho(-(float)this->getSizeX()*m_zooming*0.5, (float)this->getSizeX()*m_zooming*0.5, (float)this->getSizeY()*m_zooming*0.5, -(float)this->getSizeY()*m_zooming*0.5);
        glm::mat4 view = camera.GetViewMatrix();
        camera.SetProjection(projection);
        camera.SetView(view);

        ship.Draw(camera);
        
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

void App::loop()
{
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

void App::showFPS()
{
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

int App::getSizeX() const
{
    return m_sizeX;
}

int App::getSizeY() const
{
    return m_sizeY;
}

void App::setSizeX(int sizeX)
{
    m_sizeX = sizeX;
}

void App::setSizeY(int sizeY)
{
    m_sizeY = sizeY;
}

double App::getDeltaTime() const
{ 
    return m_delta_time;
}

double App::getTimeElapsed() const
{
    return m_chrono_elapsed;
}
