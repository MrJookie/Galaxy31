cpu_arch := 64

program_name := Galaxy31
exe := Galaxy31
tmp_binaries :=/tmp/$(program_name)
tmp_link := $(tmp_binaries)/linux$(cpu_arch)

link64bit :=    -lSDL2 \
				-lSDL2_image \
				-lSDL2_mixer \
				-lSDL2_gfx \
				-lSDL2_ttf \
				-lGL \
				-lGLEW \
				-L$(tmp_link) -lenet


link := $(link64bit) -Llibs/GUI -lgui -lcryptopp



arch := -m$(cpu_arch)

includes := -Ilibs 				 \
			-I/usr/include/SDL2  \
			-Ilibs/GUI/UI -DUSE_SDL \

hpp :=	\
		
cpp := 	\
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
		src/Quadtree.cpp								\


release := release

build := build
flags := -O2 -g
CXX := g++

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

	
.PHONY: all make_dirs extract_tmp_files

all: make_dirs libs/GUI/libgui.a $(exe) extract_tmp_files 

.ONESHELL:
extract_tmp_files:
	@if [ ! -d $(tmp_binaries)/linux$(cpu_arch) ]
	then
		mkdir -p $(tmp_binaries)/linux$(cpu_arch)
		tar -xf libs/linux$(cpu_arch).txz -C $(tmp_binaries)
	fi


windows:
	+make -f Makefile.win
	
windows_server:
	+make -f Makefile.win server

clean:
	rm -rf build
	rm -rf release
	rm -f $(exe)
	rm -rf /tmp/Galaxy31
	rm -rf /tmp/Galaxy31_server

make_dirs:
	@mkdir -p $(build)
	@mkdir -p $(build)/src/

clean_gui:
	cd libs/GUI/ && make clean
	
libs/GUI/libgui.a:
	+make -C libs/GUI/ sdl_lib
	
$(exe): $(obj)
	$(CXX) $^ -o $(exe) $(link) $(arch) -pthread
	
$(build)/%.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++14 $(arch) $(flags) $(includes)


# ----------------

make_dirs_server:
	@mkdir -p $(build)/server/src/server
server_cpp := \
		src/server/server.cpp \
		src/server/main.cpp \
		src/server/database.cpp \
		src/Object.cpp \
		
server_exe := Galaxy31_server
server_build := $(build)/server/
server_obj := $(addprefix $(server_build)/, $(patsubst %.cpp, %.o, $(server_cpp)))
server_link := -Llibs/enet-1.3.13 -lenet -lmysqlclient_r -lmysqlpp -lcryptopp
server_includes := -Ilibs/enet-1.3.13/ -I/usr/include/mysql/ -I/usr/include/mysql++/
server_flags := -Wno-deprecated-declarations -g -DSERVER

server: make_dirs_server extract_tmp_files $(server_exe)
	
$(server_build)/%.o: %.cpp
	$(CXX) -c $< -o $@ -std=c++14 $(arch) $(server_flags) $(server_includes) 
$(server_exe): $(server_obj)
	$(CXX) $^ -o $(server_exe) $(server_link) $(arch) -pthread
