#ifndef APP_HPP
#define APP_HPP

#include "Ship.hpp"
#include <chrono>
#include <iostream>

#include <vector>
#include <glm/glm.hpp>

#include "commands/bind.hpp"

class App {
    public:
        App();
        ~App();

        void init();
        void main_loop();
        void on_exit();
    private:
		void init_physics();
		void init_audio();
        void init_commands();
        void loop(); 
        void game_loop(); 
        void showFPS();
        
        
        bool server_doPassRestore(std::string user_email);
        
        void setWindowSize(glm::vec2 windowSize);
        void setScreenMousePosition(glm::vec2 screenMousePosition);
        void setWorldMousePosition(glm::vec2 worldMousePosition);
        void setZoom(float zoom);
        
        glm::vec2 getWindowSize() const;
        glm::vec2 getScreenMousePosition() const;
        glm::vec2 getWorldMousePosition() const;
        double getDeltaTime() const;
        double getTimeElapsed() const;
        float getZoom() const;
        
        void takeScreenshot(int x, int y, int w, int h);
        void takeScreenshotPNG(int x, int y, int w, int h);
        void generate_RSA_keypair();
        
        glm::vec2 m_initialWindowSize;
        glm::vec2 m_windowSize;
        glm::vec2 m_screenMousePosition;
        glm::vec2 m_worldMousePosition;
        
        int m_ticks_previous;
        int m_ticks_current;
        int m_frames_current;
        int m_frames_elapsed;
        int m_running;
        
        double m_zooming = 2;

        int m_ticks_then;
        double m_delta_time;
        
        std::chrono::high_resolution_clock::time_point m_chrono_start;
        double m_chrono_elapsed;
        
        Quadtree* m_quadtree;
        Bind m_bind;
        SDL_Window* window;
        SDL_GLContext glContext;
        SDL_DisplayMode desktopDisplayMode;
        
        void DrawObjectSelection(int x, int y, int w, int h, glm::vec4 color);
};

#endif
