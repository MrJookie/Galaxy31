#inc := -Iinclude -Isrc/Externallib
link64bit := -Llib/linux/64bit -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_gfx -lSDL2_ttf \
				-Wl,-rpath="../lib/linux/64bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW
link32bit := -Llib/linux/32bit -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_gfx -lSDL2_ttf \
				-Wl,-rpath="../lib/linux/32bit/",-rpath="../lib/linux" -Llib/linux -lGL -lGLEW
link := $(link64bit)

arch := 

hpp :=	\
		
cpp := 	\
		src/Main.cpp									\
		src/App.cpp										\

exe := Galaxy31

build := build
flags := -O2

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

.phony: make_dirs

all: make_dirs $(exe)

clean:
	find $(build) -type f -name *.o -exec rm {} \;

make_dirs:
	@mkdir -p $(build)

$(exe): $(obj)
	g++ $^ -o $(exe) $(link) $(arch) -pthread 

$(build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++14 $(arch) -pthread $(flags)
