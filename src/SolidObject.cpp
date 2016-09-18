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
bool SolidObject::DoObjectsIntersect(SolidObject* obj) {
	return !(m_position.x > obj->m_position.x + obj->m_size.x
        || m_position.x+m_size.x < obj->m_position.x
        || m_position.y > obj->m_position.y + obj->m_size.y
        || m_position.y + m_size.y < obj->m_position.y);
}

bool SolidObject::DoLinesIntersect(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4) {
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
	 
	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	if(d == 0) {
		return false;
	}
	 
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;
	 
	if(x < std::min(x1, x2) || x > std::max(x1, x2) || x < std::min(x3, x4) || x > std::max(x3, x4)) {
		return false;
	}
	
	if(y < std::min(y1, y2) || y > std::max(y1, y2) || y < std::min(y3, y4) || y > std::max(y3, y4)) {
		return false;
	}

	// point of intersection
	//return glm::vec2(x, y);

	return true;
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
