#include "Sprite.hpp"

Sprite::Sprite() {}

Sprite::Sprite(std::string imageFile)
{
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(2, vbo);
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(0);
    
	texture = this->textureFromFile(imageFile);
}

Sprite::~Sprite()
{
	glDeleteBuffers(2, vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);	
}

GLuint Sprite::textureFromFile(std::string imageFile)
{
	SDL_Surface* image = IMG_Load(imageFile.c_str());
	if(!image) {
		throw std::string("Error loading image: ") + IMG_GetError();
	}
	
	GLint colorMode;
	if(image->format->BytesPerPixel == 4) {
		if(image->format->Rmask == 0x000000ff) {
			colorMode = GL_RGBA;
		}
		else {
			colorMode = GL_BGRA;
		}
	}
	else if(image->format->BytesPerPixel == 3) {
		if(image->format->Rmask == 0x000000ff) {
			colorMode = GL_RGB;
		}
		else {
			colorMode = GL_BGR;
		}
	}
	else {
		 throw std::string("Image is not truecolor!");
	}
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel, image->w, image->h, 0, colorMode, GL_UNSIGNED_BYTE, image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);   
	 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	SDL_FreeSurface(image);
	
	return textureID;
}

void Sprite::DrawSprite(GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	glUseProgram(shader);
	glBindVertexArray(vao);
	
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	GLfloat positions[] = {
		-1.0,  1.0,
		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0,
	};
	
	GLfloat texCoords[] = {
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader, "textureUniform"), 0);

	glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
}


