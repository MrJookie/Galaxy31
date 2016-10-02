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

#include <set>
#include <vector>
class Sprite;
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
		
		struct TextureHull {
			std::string fileName;
			glm::vec2 size;
			std::vector<glm::vec2> vertices;
		};
		
		struct Shader {
			std::string fileName;
			GLuint id;
		};
		
		void LoadTexture(std::string fileName);
		Texture GetTexture(std::string fileName);
		
		void LoadTextureHull(std::string fileName);
		TextureHull GetTextureHull(std::string fileName);
		
		void LoadShader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile = "");
		Shader GetShader(std::string fileName);
		void UseShader(std::string fileName);
		void UnuseShader();
		
		Mix_Music* GetMusic(std::string fileName);
		Mix_Chunk* GetSound(std::string fileName);
		
		void AddSprite(Sprite *s);
		void RemoveSprite(Sprite* s);
		void RenderSprites();
		
		void FreeAssets();
		

	private:
		GLuint readShader(std::string shaderFile, GLenum shaderType);
		
		std::unordered_map<std::string, Texture> m_textures;
		std::unordered_map<std::string, TextureHull> m_textures_hull;
		std::unordered_map<std::string, Shader> m_shaders;
		std::unordered_map<std::string, Mix_Music*> m_musics;
		std::unordered_map<std::string, Mix_Chunk*> m_sounds;
		
		std::set<Sprite*> m_sprites;
		
};

#endif
