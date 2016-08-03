#include "Quadtree.hpp"

Quadtree::Quadtree(int left, int right, int top, int down, unsigned int maxObjects, Quadtree* parent) {
	m_left = left;
	m_right = right;
	m_top = top;
	m_down = down;
	m_maxObjects = maxObjects;
	m_parent = parent;
	m_isLeaf = true;
}

Quadtree::~Quadtree() {}

void Quadtree::DrawRect(int x, int y, int w, int h, glm::vec3 color) {
	GLuint vao, vbo_position, vbo_color;
	glGenVertexArrays(1, &vao);
	
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo_position);
    glGenBuffers(1, &vbo_color);
    
    glBindVertexArray(0);
    
	glUseProgram(GameState::asset.GetShader("shader1.vs").id);
	glBindVertexArray(vao);

    glm::mat4 modelMat;
    modelMat = glm::translate(modelMat, glm::vec3(glm::vec2(x,y), 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(glm::vec2(w,h), 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "view"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(GameState::asset.GetShader("shader1.vs").id, "projection"), 1, GL_FALSE, glm::value_ptr(GameState::camera.GetProjection()));
    
    GLfloat positions[] = {
		0.0,  1.0,
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
	};

	GLfloat colors[] = {
		 color.r, color.g, color.b,
		 color.r, color.g, color.b,
		 color.r, color.g, color.b,
		 color.r, color.g, color.b,
	};
   
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
	
	glDeleteBuffers(1, &vbo_position);
    glDeleteBuffers(1, &vbo_color);

    glDeleteVertexArrays(1, &vao);
}

void Quadtree::Draw() {
	this->DrawRect(m_left, m_top, m_right-m_left, m_down-m_top, glm::vec3(255, 0, 0));

	for(auto& child : m_children) {
		child->Draw();
	}
}

void Quadtree::AddObject(Object* object) {
	if(m_isLeaf) {
		m_objects.push_back(object);

		if(m_objects.size() == m_maxObjects + 1) {
			this->createLeaves();
			this->moveObjectsToLeaves();
		}
		
		return;
	}
	
	for(auto& child : m_children) {
		if(child->contains(object)) {
			child->AddObject(object);
		}
	}
}

void Quadtree::Clear() {
	m_objects.clear();

	for(auto& child : m_children) {
		//child->Clear();
		child.reset();
	}
	
	m_isLeaf = true;
}

void Quadtree::QueryRectangle(int x, int y, int w, int h, std::unordered_map<Object*, Quadtree*>& returnObjects) {
	if(intersects(x, y, w, h) ) {	
		for(auto& object : m_objects) {
				if(intersects(x, y, w, h)) {
					returnObjects[object] = m_parent;
				}
		}
		
		for(auto& child : m_children) {
			child->QueryRectangle(x, y, w, h, returnObjects);
		}
	}
}

bool Quadtree::contains(Object* object) {
	int x = object->GetPosition().x - object->GetSize().x/2;
	int y = object->GetPosition().y - object->GetSize().y/2;
	int w = object->GetSize().x;
	int h = object->GetSize().y;
	
	if(x > m_left && x < m_right && y > m_top && y < m_down) {
		return true;
	} else if(x+w > m_left && x+w < m_right && y > m_top && y < m_down) {
		return true;
	} else if (x > m_left && x < m_right && y+h > m_top && y+h < m_down) {
		return true;
	} else if (x+w > m_left && x+w < m_right && y+h > m_top && y+h < m_down) {
		return true;
	}
	
	return false;
}

bool Quadtree::intersects(int x, int y, int w, int h) {
	return !(m_down < y || m_top > y+h || m_right < x || m_left > x+w);
}

void Quadtree::createLeaves() {
	m_children.resize(4);
	m_children[0] = std::make_unique<Quadtree>(m_left, (m_left+m_right)/2, m_top, (m_top+m_down)/2, m_maxObjects, this);
	m_children[1] = std::make_unique<Quadtree>((m_left+m_right)/2, m_right, m_top, (m_top+m_down)/2, m_maxObjects, this);
	m_children[2] = std::make_unique<Quadtree>(m_left, (m_left+m_right)/2, (m_top+m_down)/2, m_down, m_maxObjects, this);
	m_children[3] = std::make_unique<Quadtree>((m_left+m_right)/2, m_right, (m_top+m_down)/2, m_down, m_maxObjects, this);
	
	m_isLeaf = false;
}

void Quadtree::moveObjectsToLeaves() {
	while(m_objects.size()) {
		for(auto& child : m_children) {
			if(child->contains(m_objects[0])) {
				child->AddObject(m_objects[0]);
			}
		}
		
		m_objects.erase(m_objects.begin());
	}
}
