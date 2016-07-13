#inc := -Iinclude -Isrc/Externallib
link64bit := -Llib/linux/64bit -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_gfx -lSDL2_ttf \
				-Wl,-rpath="../lib/linux/64bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW
				
link32bit := -Llib/linux/32bit -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_gfx -lSDL2_ttf \
				-Wl,-rpath="../lib/linux/32bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW
				
link := $(link64bit)

arch := 

includes := -Ilibs 				 \
			-Ilibs/imgui 		 \
			-I/usr/include/SDL2

ImGui := libs/imgui/imgui_impl_sdl_gl3.cpp  \
		 libs/imgui/imgui.cpp				\
		 libs/imgui/imgui_draw.cpp

hpp :=	\
		
cpp := 	\
		$(ImGui)										\
		src/Main.cpp									\
		src/GameState.cpp								\
		src/App.cpp										\
		src/Asset.cpp									\
		src/Object.cpp									\
		src/Camera.cpp									\
		src/Sprite.cpp									\
		src/Ship.cpp									\

exe := Galaxy31

build := build
flags := -O2

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

.phony: make_dirs

all: make_dirs $(exe)

clean:
	find $(build) -type f -name *.o -exec rm {} \;
	rm -f $(exe)

make_dirs:
	@mkdir -p $(build)
	@mkdir -p $(build)/src/
	@mkdir -p $(build)/libs/imgui/

$(exe): $(obj)
	g++ $^ -o $(exe) $(link) $(arch) -pthread

$(build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++14 $(arch) -pthread $(flags) $(includes)
