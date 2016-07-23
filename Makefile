#inc := -Iinclude -Isrc/Externallib
link64bit :=    -lSDL2 \
				-lSDL2_image \
				-lSDL2_mixer \
				-lSDL2_gfx \
				-lSDL2_ttf \
				-lGL \
				-lGLEW \
				-Llibs/enet-1.3.13 -lenet


link := $(link64bit)

arch := 

includes := -Ilibs 				 \
			-Ilibs/imgui 		 \
			-I/usr/include/SDL2	 \
			-Ilibs/enet-1.3.13/

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
		src/Projectile.cpp								\
		src/Network.cpp									\

exe := Galaxy31

build := build
flags := -O2

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

.phony: make_dirs

all: make_dirs $(exe)


clean:
	rm -rf build
	rm -rf release
	rm -f $(exe)

make_dirs:
	@mkdir -p $(build)
	@mkdir -p $(build)/src/
	@mkdir -p $(build)/libs/imgui/

$(exe): $(obj)
	g++ $^ -o $(exe) $(link) $(arch) -pthread

$(build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++14 $(arch) $(flags) $(includes)


# ----------------
.ONESHELL:
make_dirs_server:
	@mkdir -p $(build)/server/src/server
server_cpp := \
		src/server/server.cpp \
		src/server/main.cpp
server_exe := galaxy31_server
server_build := $(build)/server/
server_obj := $(addprefix $(server_build)/, $(patsubst %.cpp, %.o, $(server_cpp)))
server_link := -Llibs/enet-1.3.13 -lenet
server_includes := -Ilibs/enet-1.3.13/
server_arch := -m64

server: make_dirs_server $(server_exe)
	
$(server_build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++14 $(server_arch) $(server_includes) 
$(server_exe): $(server_obj)
	g++ $^ -o $(server_exe) $(server_link) $(server_arch) -pthread
