#include "Asset.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "GameState.hpp"

Asset::Asset() {}

Asset::~Asset() {}

void Asset::AddSprite(Sprite *s) {
	m_sprites.insert(s);
}
void Asset::RemoveSprite(Sprite *s) {
	m_sprites.erase(s);
}
GLuint m_vao, m_vbo, m_ebo;
bool inited = false;
void Asset::RenderSprites() {
	GameState::objectsDrawn+=m_sprites.size();
	
	if(!inited) {
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);
		inited = true;
		
		glBindVertexArray(m_vao);
		GLfloat position_and_texcoords[] = {
			0.0,  1.0,
			0.0,  0.0,
			1.0,  0.0,
			1.0,  1.0,
		};

		GLuint indices[] = {
			0, 1, 3,
			1, 2, 3,
		};
		
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(position_and_texcoords), position_and_texcoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}
	GLuint shader = GameState::asset.GetShader("sprite.vs").id;
	glBindVertexArray(m_vao);
	
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_ADD);
    
    glUseProgram(shader);
    glBindVertexArray(m_vao);
	
    //glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetViewMatrix()));
    //glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetProjectionMatrix()));
    glm::mat4 projectionViewMat = GameState::camera.GetProjectionMatrix() * GameState::camera.GetViewMatrix();
    //glUniformMatrix4fv(glGetUniformLocation(shader, "projection_view_matrix"), 1, GL_FALSE, glm::value_ptr(projectionViewMat));
    
    //GLuint model = glGetUniformLocation(shader, "model");
	GLuint tex_uniform = glGetUniformLocation(shader, "textureUniform");
	GLuint texSkin_uniform = glGetUniformLocation(shader, "textureSkinUniform");
        
    for(auto& o : m_sprites) {
		Sprite& s = *o;
		const glm::vec2& size = s.GetSize();
		const glm::vec2& position = s.GetPosition();
		const float rotation = s.GetRotation();
		const std::vector<GLuint>& textures = s.GetTextures();
		if(textures.empty()) continue;

		glm::mat4 modelMat;
		modelMat = glm::translate(modelMat, glm::vec3(position + size * 0.5f, 0.0f) );
		modelMat = glm::rotate(modelMat, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMat = glm::translate(modelMat, glm::vec3(size * -0.5f, 0.0f));
		modelMat = glm::scale(modelMat, glm::vec3(size, 1.0f));
		
		/*
		glm::mat3 modelMat;
		modelMat = glm::translate(modelMat, position + size * 0.5f );
		modelMat = glm::rotate(modelMat, glm::radians(rotation));
		modelMat = glm::translate(modelMat, size * -0.5f);
		modelMat = glm::scale(modelMat, size);
		*/
		
		//glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(modelMat));
		//glUniformMatrix3fv(model, 1, GL_FALSE, glm::value_ptr(modelMat));
		
		glm::mat4 mvp = projectionViewMat * modelMat;
		glUniformMatrix4fv(glGetUniformLocation(shader, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

		// setting textures in texture units
		// int i=0;
		// for(const auto tex : m_textures) {
			// glActiveTexture(GL_TEXTURE0+(i++));
			// glBindTexture(GL_TEXTURE_2D, tex);
		// }
		
		glUniform1i(tex_uniform, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		bool hasSkin = false;
		
		//has skin
		if(textures.size() > 1 && glIsTexture(textures[1])) {
			glUniform1i(texSkin_uniform, 1);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, textures[1]);
			hasSkin = true;
		}
		
		glUniform1i(glGetUniformLocation(shader, "hasSkin"), hasSkin);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	
    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
    
    //glDisable(GL_TEXTURE_2D);
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
		//m_textures[fileName].image = image;

		SDL_FreeSurface(image);
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

void Asset::LoadTextureHull(std::string fileName) {
	if(!m_textures_hull[fileName].fileName.length() > 0) {
		SDL_Surface* image = IMG_Load((TEXTURE_PATH + fileName).c_str());
		if(!image) {
			throw std::string("Error loading image: ") + IMG_GetError();
		}

		std::vector<glm::vec2> orderedVertices;
		std::map<double, glm::vec2> vertices;
		
		glm::vec2 texCenter(image->h / 2, image->w / 2);
		
		Uint32 *pixels = (Uint32 *)image->pixels;
		
		for(int x = 0; x < image->w; ++x) {
			for(int y = 0; y < image->h; ++y) {
				Uint32 color = pixels[(y * image->w) + x];
				
				int ca = (color >> 24) & 0xff;
				int cb = (color >> 16) & 0xff;
				int cg = (color >> 8) & 0xff;
				int cr = color & 0xff;
				
				//magenta
				if(cr == 255 && cg == 0 && cb == 255) {
					double angleBetweenVectors = atan2(y - texCenter.y, x - texCenter.x);
					vertices[angleBetweenVectors] = glm::vec2(x,y);
				}
			}
		}
		
		for(auto& v : vertices) {
			orderedVertices.push_back(v.second);
			
			//std::cout << "btVector3(" << v.second.x - 276/2 << ", " << v.second.y - 276/2 << ", 0)," << std::endl;
		}
		
		m_textures_hull[fileName].fileName = fileName;
		m_textures_hull[fileName].size = glm::vec2(image->w, image->h);
		m_textures_hull[fileName].vertices = orderedVertices;
		
		SDL_FreeSurface(image);
	}
}

Asset::TextureHull Asset::GetTextureHull(std::string fileName) {
	auto texture_it = m_textures_hull.find(fileName);
	if(texture_it == m_textures_hull.end()) {
		LoadTextureHull(fileName);
	}
	
	return m_textures_hull[fileName];
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
		//SDL_FreeSurface(texture.second.image);
	}
	
	for(const auto& shader : m_shaders) {
		glDeleteProgram(shader.second.id);
	}
	
	for(const auto& music : m_musics) {
		Mix_FreeMusic(music.second);
	}
}
