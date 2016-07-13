#include "Sprite.hpp"

Sprite::Sprite() {
    m_texture = 0;
    
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(2, m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(0);
}

Sprite& Sprite::operator=(Sprite && o) {
    *this = o;
    o.m_texture = 0;
}

Sprite::~Sprite() {
    if(m_texture == 0) return;
    glDeleteBuffers(2, m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void Sprite::SetTexture(GLuint textureID) {
	m_texture = textureID;
}

void Sprite::Draw() {
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glUseProgram(GameState::asset.GetShader("Assets/Shaders/sprite.vs").id);
    glBindVertexArray(m_vao);

    m_modelMat = glm::mat4(1.0);

    m_modelMat = glm::translate(m_modelMat, glm::vec3(m_position, 0.0f));

    m_modelMat = glm::translate(m_modelMat, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.0f));
    m_modelMat = glm::rotate(m_modelMat, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    m_modelMat = glm::translate(m_modelMat, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.0f));

    m_modelMat = glm::scale(m_modelMat, glm::vec3(m_size, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/sprite.vs").id, "model"), 1, GL_FALSE, glm::value_ptr(m_modelMat));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/sprite.vs").id, "view"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/sprite.vs").id, "projection"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetProjection()));

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
    glUniform1i(glGetUniformLocation(GameState::asset.GetShader("Assets/Shaders/sprite.vs").id, "textureUniform"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
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
