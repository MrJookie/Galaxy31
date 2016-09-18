#include "SolidObject.hpp"
#include "GameState.hpp"

void SolidObject::UpdateHullVertices(std::vector<glm::vec2> hullVertices) {
	glm::mat4 modelMat;
    modelMat = glm::translate(modelMat, glm::vec3(m_position, 0.0f));

    modelMat = glm::translate(modelMat, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMat = glm::translate(modelMat, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.0f));
    
    std::vector<glm::vec2> transformedVerts;
    for(int i = 0; i < hullVertices.size(); ++i) {
		transformedVerts.push_back(glm::vec2(modelMat * glm::vec4(hullVertices[i], 1.0, 1.0)));
	}
	
	m_hullVertices = transformedVerts;
}

std::vector<glm::vec2> SolidObject::GetCollisionHull() {
	return m_hullVertices;
}

void SolidObject::RenderCollisionHull() {
	GLuint vao, vbo[2];
	glGenVertexArrays(1, &vao);
	
    glBindVertexArray(vao);
    glGenBuffers(2, vbo);
    
	glUseProgram(GameState::asset.GetShader("shader1.vs").id);

    glm::mat4 modelMat(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "view"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "projection"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetProjection()));

	std::vector<glm::vec4> colors;
	for(int i = 0; i < m_hullVertices.size(); ++i) {
		colors.push_back(CollisionHullColor);
	}
   
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * m_hullVertices.size(), &m_hullVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * colors.size(), &colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glDrawArrays(GL_LINE_LOOP, 0, m_hullVertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
	
    glDeleteBuffers(2, vbo);

    glDeleteVertexArrays(1, &vao);
}
