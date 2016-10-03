#include "Sprite.hpp"
#include "GameState.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Asset.hpp"
#include <iostream>
using std::cout;
using std::endl;
int shader;
bool Sprite::first_time = true;
Sprite::Sprite() : m_size(0), m_position(0), m_rotation(0) {
    added = false;
    if(first_time) {
		shader = GameState::asset.GetShader("sprite.vs").id;
		first_time = false;
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);

		glBindVertexArray(0);
	}
}


GLuint Sprite::m_vao;
GLuint Sprite::m_vbo;
GLuint Sprite::m_ebo;

Sprite& Sprite::operator=(Sprite && o) {
    m_vao = o.m_vao; m_vbo = o.m_vbo;
    m_ebo = o.m_ebo; m_size = o.m_size;
    m_position = o.m_position;
    m_textures = o.m_textures;
    o.m_textures.clear();
    added = false;
}

Sprite::~Sprite() {
    if(m_textures.size() == 0) return;
    GameState::asset.RemoveSprite(this);
    // cout << "removing " << this << endl;
    // glDeleteBuffers(1, &m_vbo);
    // glDeleteBuffers(1, &m_ebo);
    // glDeleteVertexArrays(1, &m_vao);
}

void Sprite::SetTexture(const Asset::Texture &tex) {
	m_size = tex.size;
	AddTexture(tex.id);
}
void Sprite::AddTexture(GLuint textureID) {
	m_textures.push_back(textureID);
}

void Sprite::DrawSprite(glm::vec2 size, glm::vec2 position, float rotation) {
	m_size = size;
	m_position = position;
	m_rotation = rotation;
	DrawSprite();
}

void Sprite::RemoveFromDrawing() {
	added = false;
	GameState::asset.RemoveSprite(this);
}

void Sprite::DrawSprite() {
	if(!added && !m_textures.empty()) {
		GameState::asset.AddSprite(this);
		added = true;
		// cout << "adding " << this << endl;
	}
	//return;
	/*
	GameState::objectsDrawn++;
	
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glUseProgram(shader);
    glBindVertexArray(m_vao);

	
    glm::mat4 modelMat;
    modelMat = glm::translate(modelMat, glm::vec3(m_position, 0.0f));

    modelMat = glm::translate(modelMat, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMat = glm::translate(modelMat, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.0f));

    modelMat = glm::scale(modelMat, glm::vec3(m_size, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetProjection()));

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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // setting textures in texture units
    int i=0;
    for(auto tex : m_textures) {
		glActiveTexture(GL_TEXTURE0+(i++));
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	
    glUniform1i(glGetUniformLocation(shader, "textureUniform"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
    */
}

void Sprite::SetSize(glm::vec2 size) {
    m_size = size;
}

void Sprite::SetPosition(glm::vec2 position) {
    m_position = position;
}

void Sprite::SetRotation(float rotation) {
    m_rotation = rotation;
}

glm::vec2 Sprite::GetSize() const {
    return m_size;
}

glm::vec2 Sprite::GetPosition() const {
    return m_position;
}

float Sprite::GetRotation() const {
    return m_rotation;
}
