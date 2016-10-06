#include "GLDebugDrawer.hpp"

#include <iostream>

GLDebugDrawer::GLDebugDrawer(int sizeX, int sizeY)
: m_debugMode(0), m_sizeX(sizeX), m_sizeY(sizeY)
{
	glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(2, m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(0);
}

void GLDebugDrawer::Render(GLuint shader, glm::mat4 view, glm::mat4 projection) {
	GLuint vao, vbo[2], ebo;
	
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);
	
	glUseProgram(shader);
	glBindVertexArray(vao);

	glm::mat4 modelMat(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m_colors.size(), &m_colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glDrawArrays(GL_LINES, 0, m_vertices.size() * 2);

    glBindVertexArray(0);
    glUseProgram(0);
    
    glDeleteBuffers(2, vbo);
    glDeleteVertexArrays(1, &vao);
    
    m_vertices.clear();
    m_colors.clear();
}

void GLDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
	int xA = from.getX();
	int yA = from.getY();
	int xB = to.getX();
	int yB = to.getY();
	
	//std::cout << from.getX() << " : " << from.getY() << " : " << from.getZ() << " : " << to.getX() << " --- " << to.getY() << " --- " << to.getZ() << std::endl;
    
	m_vertices.push_back(glm::vec2(from.getX(), from.getY()));
	m_vertices.push_back(glm::vec2(to.getX(), to.getY()));
	
	m_colors.push_back(glm::vec4(fromColor.getX(), fromColor.getY(), fromColor.getZ(), 1.0));
	m_colors.push_back(glm::vec4(toColor.getX(), toColor.getY(), toColor.getZ(), 1.0));
}

void GLDebugDrawer::drawLine (const btVector3& from, const btVector3& to, const btVector3& color)
{
  drawLine(from, to, color, color);
}

void GLDebugDrawer::drawSphere (const btVector3& p, btScalar radius, const btVector3& color)
{
	std::cout << "drawSphere" << std::endl;
}

void GLDebugDrawer::drawBox (const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha)
{
	std::cout << "drawBox" << std::endl;
}

void GLDebugDrawer::drawTriangle (const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha)
{
	std::cout << "drawTriangle" << std::endl;
}

void GLDebugDrawer::draw3dText (const btVector3& location, const char* string)
{
}

void GLDebugDrawer::reportErrorWarning (const char* string)
{
}

void GLDebugDrawer::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int /*lifeTime*/, const btVector3& color)
{
	std::cout << "drawContactPoint" << std::endl;
}




