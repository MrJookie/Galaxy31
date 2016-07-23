#ifndef ASSET_HPP
#define ASSET_HPP

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
//~ #include <GL/gl3w.h>

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//later pack files and access just by filenames and hardcode filepaths

class Asset {
	public:
		Asset();
		~Asset();
		
		struct Texture {
			std::string fileName;
			GLuint id;
			glm::vec2 size;
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
		
		
		

	private:
		void FreeAssets();
		GLuint readShader(std::string shaderFile, GLenum shaderType);
		
		std::unordered_map<std::string, Texture> m_textures;
		std::unordered_map<std::string, Shader> m_shaders;
		std::unordered_map<std::string, Mix_Music*> m_musics;
		std::unordered_map<std::string, Mix_Chunk*> m_sounds;
		
};

#endif
