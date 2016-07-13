#ifndef APP_HPP
#define APP_HPP

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
//~ #include <GL/gl3w.h>
#include <GL/glew.h>

#include "GameState.hpp"
#include "Asset.hpp"
#include "Object.hpp"
#include "Camera.hpp"
#include "Sprite.hpp"
#include "Ship.hpp"

class App {
    public:
        App();
        ~App();

    private:
        void init();
        void loop(); 
        void showFPS();
        
        void setWindowSize(glm::vec2 windowSize);
        void setScreenMousePosition(glm::vec2 screenMousePosition);
        void setWorldMousePosition(glm::vec2 worldMousePosition);
        
        glm::vec2 getWindowSize() const;
        glm::vec2 getScreenMousePosition() const;
        glm::vec2 getWorldMousePosition() const;
        double getDeltaTime() const;
        double getTimeElapsed() const;
        
        void takeScreenshot(int x, int y, int w, int h);
        void takeScreenshotPNG(int x, int y, int w, int h);
        
        glm::vec2 m_windowSize;
        glm::vec2 m_screenMousePosition;
        glm::vec2 m_worldMousePosition;
        
        int m_ticks_previous;
        int m_ticks_current;
        int m_frames_current;
        int m_frames_elapsed;
        
        float m_zooming = 2;

        int m_ticks_then;
        double m_delta_time;
        
        std::chrono::high_resolution_clock::time_point m_chrono_start;
        double m_chrono_elapsed;
};

#endif
