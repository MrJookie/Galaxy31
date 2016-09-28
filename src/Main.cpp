#define SDL_MAIN_HANDLED
#include "App.hpp"
#include "commands/commands.hpp"

int main(int argc, char* args[]) {
	App app;
    try {
        app.init();
    } catch(std::string& error) {
        std::cout << error << std::endl;
        return -1;
    }
    app.main_loop();
    app.on_exit();
    

    return 0;
}
