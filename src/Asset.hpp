#ifndef ASSET_HPP
#define ASSET_HPP

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	private:
		GLuint readShader(std::string shaderFile, GLenum shaderType);
		
		std::unordered_map<std::string, Texture> m_textures;
		std::unordered_map<std::string, Shader> m_shaders;
		//std::unordered_map<std::string, Sound> m_sounds;
		
};

#endif