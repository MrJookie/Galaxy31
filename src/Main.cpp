#define SDL_MAIN_HANDLED
#include "App.hpp"

int main(int argc, char* args[]) {
    try {
        App app;
    } catch(std::string& error) {
        std::cout << error << std::endl;
        return -1;
    }

    return 0;
}
