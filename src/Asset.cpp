#include "Asset.hpp"

Asset::Asset() {}

Asset::~Asset() {
		for(const auto& texture : m_textures) {
			glDeleteTextures(1, &texture.second.id);
		}
		
		for(const auto& shader : m_shaders) {
			glDeleteProgram(shader.second.id);
		}
}

void Asset::LoadTexture(std::string fileName) {
	if(!m_textures[fileName].fileName.length() > 0) { //load only if texture fileName is not loaded yet, not calling GetTexture == 0, because it would throw error
		SDL_Surface* image = IMG_Load(fileName.c_str());
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

		SDL_FreeSurface(image);
	} else {
		//std::cout << "Texture: " << fileName << " already loaded!" << std::endl;
	}
}

Asset::Texture Asset::GetTexture(std::string fileName) {
	if(!m_textures[fileName].fileName.length() > 0) {
		throw std::string("Trying to access unitialized texture: ") + fileName;
	}
	
	return m_textures[fileName];
}


void Asset::LoadShader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile) {
	if(!m_shaders[vertexShaderFile].fileName.length() > 0) {
		GLuint vertexShader = this->readShader(vertexShaderFile, GL_VERTEX_SHADER);
		GLuint geometryShader = 0;
		if(geometryShaderFile.length() > 0) {
			geometryShader = this->readShader(geometryShaderFile, GL_GEOMETRY_SHADER);
		}
		GLuint fragmentShader = this->readShader(fragmentShaderFile, GL_FRAGMENT_SHADER);

		// linking
		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		if(geometryShaderFile.length() > 0) {
			glAttachShader(shaderProgram, geometryShader);
		}
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);

		if(!shaderProgram) {
			throw std::string("Failed to create shader program");
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

        throw std::string(buffer);
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

