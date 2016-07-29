#include "Asset.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Asset::Asset() {}

#define SHADER_PATH "Assets/Shaders/"
#define TEXTURE_PATH "Assets/Textures/"
#define MUSIC_PATH "Assets/Music/"
#define SOUND_PATH "Assets/Sound/"

Asset::~Asset() {
}

void Asset::LoadTexture(std::string fileName) {
	if(!m_textures[fileName].fileName.length() > 0) { //load only if texture fileName is not loaded yet, not calling GetTexture == 0, because it would throw error
		SDL_Surface* image = IMG_Load((TEXTURE_PATH + fileName).c_str());
		if(!image) {
			throw std::string("Error loading image: ") + IMG_GetError();
		}

		GLint colorMode;
		if(image->format->BytesPerPixel == 4) {
			if(image->format->Rmask == 0x000000ff) {
				colorMode = GL_RGBA;
			} else {
				colorMode = GL_BGRA;
			}
		} else if(image->format->BytesPerPixel == 3) {
			if(image->format->Rmask == 0x000000ff) {
				colorMode = GL_RGB;
			} else {
				colorMode = GL_BGR;
			}
		} else {
			throw std::string("Image is not truecolor!");
		}
		
		GLuint textureID;
		glGenTextures(1, &textureID);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, colorMode, GL_UNSIGNED_BYTE, image->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		m_textures[fileName].fileName = fileName;
		m_textures[fileName].id = textureID;
		m_textures[fileName].size = glm::vec2(image->w, image->h);
		m_textures[fileName].image = image;

		//SDL_FreeSurface(image);
	} else {
		//std::cout << "Texture: " << fileName << " already loaded!" << std::endl;
	}
}

Asset::Texture Asset::GetTexture(std::string fileName) {
	auto texture_it = m_textures.find(fileName);
	if(texture_it == m_textures.end()) {
		LoadTexture(fileName);
	}
	
	return m_textures[fileName];
}

void Asset::LoadShader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile) {
	std::string vertex = std::string(SHADER_PATH) + vertexShaderFile;
	std::string fragment = std::string(SHADER_PATH) + fragmentShaderFile;
	std::string geometry = std::string(SHADER_PATH) + geometryShaderFile;
	if(!m_shaders[vertexShaderFile].fileName.length() > 0) {
		GLuint vertexShader = this->readShader(vertex, GL_VERTEX_SHADER);
		GLuint geometryShader = 0;
		if(geometryShaderFile.length() > 0) {
			geometryShader = this->readShader(geometry, GL_GEOMETRY_SHADER);
		}
		GLuint fragmentShader = this->readShader(fragment, GL_FRAGMENT_SHADER);

		// linking
		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		if(geometryShaderFile.length() > 0) {
			glAttachShader(shaderProgram, geometryShader);
		}
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);

		if(!shaderProgram) {
			throw std::string("Failed to create shader program: ") + vertexShaderFile;
		}
		
		m_shaders[vertexShaderFile].fileName = vertexShaderFile;
		m_shaders[vertexShaderFile].id = shaderProgram;
	}
}

GLuint Asset::readShader(std::string shaderFile, GLenum shaderType) {
    // reading shader
    std::ifstream shaderStream(shaderFile);
    if(!shaderStream) {
        throw std::string("Failed to load shader file: ") + shaderFile;
    }

    std::stringstream shaderData;

    shaderData << shaderStream.rdbuf();
    shaderStream.close();

    const std::string &shaderString = shaderData.str();
    const char *shaderSource = shaderString.c_str();
    GLint shaderLength = shaderString.size();

    // creating shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar**)&shaderSource, (GLint*)&shaderLength);

    // compiling shader
    GLint compileStatus;

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if(compileStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);

        throw "Shader " + shaderFile + "  Error: " + std::string(buffer);
    }

    return shader;
}

Asset::Shader Asset::GetShader(std::string fileName) {
	if(!m_shaders[fileName].fileName.length() > 0) {
		throw std::string("Trying to access unitialized shader: ") + fileName;
	}
	
	return m_shaders[fileName];
}

void Asset::UseShader(std::string fileName) {
    glUseProgram(this->GetShader(fileName).id);
}

void Asset::UnuseShader() {
    glUseProgram(0);
}

Mix_Music* Asset::GetMusic(std::string fileName) {
	auto music_it = m_musics.find(fileName);
	if( music_it != m_musics.end()) {
		return music_it->second;
	} else {
		Mix_Music *music = Mix_LoadMUS((MUSIC_PATH + fileName).c_str());
		if(music) {
			m_musics[fileName] = music;
		} else {
			throw std::string("Failed to load music file: ") + fileName;
		}
		return music;
	}
}

Mix_Chunk* Asset::GetSound(std::string fileName) {
	auto sound_it = m_sounds.find(fileName);
	if( sound_it != m_sounds.end()) {
		return sound_it->second;
	} else {
		Mix_Chunk *sound = Mix_LoadWAV((SOUND_PATH + fileName).c_str());
		if(sound) {
			m_sounds[fileName] = sound;
		} else {
			throw std::string("Failed to load sound file: ") + fileName;
		}
		return sound;
	}
}

void Asset::FreeAssets() {
	for(const auto& texture : m_textures) {
		glDeleteTextures(1, &texture.second.id);
		SDL_FreeSurface(texture.second.image);
	}
	
	for(const auto& shader : m_shaders) {
		glDeleteProgram(shader.second.id);
	}
	
	for(const auto& music : m_musics) {
		Mix_FreeMusic(music.second);
	}
}
