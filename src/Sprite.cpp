#include "Sprite.hpp"

Sprite::Sprite() {}

Sprite::Sprite(std::string imageFile)
{
	glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(2, m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(0);
	m_texture = this->textureFromFile(imageFile);
}

Sprite::~Sprite()
{
	glDeleteBuffers(2, m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, colorMode, GL_UNSIGNED_BYTE, image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	this->SetSize(image->w, image->h);
	
	SDL_FreeSurface(image);
	
	return textureID;
}

void Sprite::DrawSprite(GLuint shader, glm::mat4 view, glm::mat4 projection)
{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glUseProgram(shader);
	glBindVertexArray(m_vao);
	
	m_modelMat = glm::mat4(1.0);
	
	m_modelMat = glm::translate(m_modelMat, glm::vec3(m_position, 0.0f));  

	m_modelMat = glm::translate(m_modelMat, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.0f));
	m_modelMat = glm::rotate(m_modelMat, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 
	m_modelMat = glm::translate(m_modelMat, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.0f));
	
	m_modelMat = glm::scale(m_modelMat, glm::vec3(m_size, 1.0f));
	
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(m_modelMat));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	GLfloat positions[] = {
		 0.0,  1.0,
		 0.0,  0.0,
		 1.0,  0.0,
		 1.0,  1.0,
	};
	
	GLfloat texCoords[] = {
		 0.0,  1.0,
		 0.0,  0.0,
		 1.0,  0.0,
		 1.0,  1.0,
	};
	
	GLuint indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glUniform1i(glGetUniformLocation(shader, "textureUniform"), 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

	glDisable(GL_BLEND);
}

void Sprite::SetSize(int sizeX, int sizeY)
{
	m_size.x = sizeX;
	m_size.y = sizeY;
}

void Sprite::SetPosition(float posX, float posY)
{
	m_position.x = posX;
	m_position.y = posY;
}

void Sprite::SetRotation(float rotation)
{
	m_rotation = rotation;
}

glm::vec2 Sprite::GetSize()
{
	return m_size;
}

glm::vec2 Sprite::GetPosition()
{
	return m_position;
}

float Sprite::GetRotation()
{
	return m_rotation;
}
