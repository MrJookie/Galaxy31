#ifndef ASSET_HPP
#define ASSET_HPP

//opengl
#include <GL/glew.h>

//sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

//stl
#include <unordered_map>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//gui
#include "Gui.hpp"
#include "Control.hpp"
#include "controls/Button.hpp"
#include "controls/ScrollBar.hpp"
#include "controls/Container.hpp"
#include "controls/ComboBox.hpp"
#include "controls/GridContainer.hpp"
#include "controls/TextBox.hpp"
#include "controls/RadioButton.hpp"
#include "controls/ListBox.hpp"
#include "controls/Label.hpp"
#include "controls/TrackBar.hpp"
#include "controls/Canvas.hpp"
#include "controls/CheckBox.hpp"
#include "controls/WidgetMover.hpp"
#include "controls/Terminal.hpp"
#include "controls/Form.hpp"
#include "common/SDL/Drawing.hpp"

//later pack files and access just by filenames and hardcode filepaths

class Asset {
	public:
		Asset();
		~Asset();
		
		struct Texture {
			std::string fileName;
			GLuint id;
			glm::vec2 size;
			//SDL_Surface* image;
		};
		
		struct Shader {
			std::string fileName;
			GLuint id;
		};
		
		void LoadTexture(std::string fileName);
		Texture GetTexture(std::string fileName);
		
		void LoadShader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile = "");
		Shader GetShader(std::string fileName);
		void UseShader(std::string fileName);
		void UnuseShader();
		
		Mix_Music* GetMusic(std::string fileName);
		Mix_Chunk* GetSound(std::string fileName);
		
		
		void FreeAssets();
		

	private:
		GLuint readShader(std::string shaderFile, GLenum shaderType);
		
		std::unordered_map<std::string, Texture> m_textures;
		std::unordered_map<std::string, Shader> m_shaders;
		std::unordered_map<std::string, Mix_Music*> m_musics;
		std::unordered_map<std::string, Mix_Chunk*> m_sounds;
		
};

#endif
