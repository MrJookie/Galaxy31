#include "Quadtree.hpp"

static int nodesSum = 0;

Quadtree::Quadtree(int left, int right, int top, int down, unsigned int maxObjects, Quadtree* parent, int level, int maxLevel) {
	m_left = left;
	m_right = right;
	m_top = top;
	m_down = down;
	m_maxObjects = maxObjects;
	m_parent = parent;
	m_isLeaf = true;
	
	m_level = level;
	m_maxLevel = maxLevel;
	nodesSum++;
	
	std::cout << nodesSum << std::endl;
}

Quadtree::~Quadtree() {}

void Quadtree::DrawRect(int x, int y, int w, int h, glm::vec4 color) {
	GLuint vao, vbo[2];
	glGenVertexArrays(1, &vao);
	
    glBindVertexArray(vao);
    glGenBuffers(2, vbo);
    
	glUseProgram(GameState::asset.GetShader("shader1.vs").id);

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
		 color.r, color.g, color.b, color.a,
		 color.r, color.g, color.b, color.a,
		 color.r, color.g, color.b, color.a,
		 color.r, color.g, color.b, color.a,
	};
   
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);    
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);    
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
	
	glDeleteBuffers(2, vbo);

    glDeleteVertexArrays(1, &vao);
}

void Quadtree::Draw() {
	this->DrawRect(m_left, m_top, m_right-m_left, m_down-m_top, glm::vec4(1, 0, 0, 1));

	for(auto& child : m_children) {
		child->Draw();
	}
}

void Quadtree::AddObject(Object* object) {
	//if(m_level == m_maxLevel) {
	if(m_isLeaf) {
		m_objects.push_back(object);
		((SolidObject*)object)->NodePtr = m_parent;
		//((SolidObject*)object)->NodePtr = this;
		
		return;
	}

	for(auto& child : m_children) {
		if(child->contains(object)) {
			child->AddObject(object);
		}
	}
}

void Quadtree::Resize() {
	this->createLeaves();
	
	for(auto& child : m_children) {
		child->Resize();
	}
}

void Quadtree::Clear() {
	m_objects.clear();
	
	for(auto& child : m_children) {
		child->Clear();
	}
}

void Quadtree::QueryRectangle(int x, int y, int w, int h, std::vector<Object*>& returnObjects) {
	if(intersects(x, y, w, h) ) {	
		for(auto& object : m_objects) {
			returnObjects.push_back(object);
		}
		
		for(auto& child : m_children) {
			child->QueryRectangle(x, y, w, h, returnObjects);
		}
	}
}

void Quadtree::GetObjectsInNode(std::vector<Object*>& returnObjects) {
	if(!m_isLeaf) {
		//std::vector<Object*> objects;
		
		for(auto& child : m_children) {
			//if(intersects->contains(x, y, w, h) //todo
			for(auto& object : child->m_objects) {
				returnObjects.push_back(object);
				//objects.push_back(object);
			}
		}
		
		//return;
	}
	
	//return m_objects;
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
	if (m_level == m_maxLevel) {
		return;
	}
	
	m_children.resize(4);
	m_children[0] = std::make_unique<Quadtree>(m_left, (m_left+m_right)/2, m_top, (m_top+m_down)/2, m_maxObjects, this, m_level+1, m_maxLevel);
	m_children[1] = std::make_unique<Quadtree>((m_left+m_right)/2, m_right, m_top, (m_top+m_down)/2, m_maxObjects, this, m_level+1, m_maxLevel);
	m_children[2] = std::make_unique<Quadtree>(m_left, (m_left+m_right)/2, (m_top+m_down)/2, m_down, m_maxObjects, this, m_level+1, m_maxLevel);
	m_children[3] = std::make_unique<Quadtree>((m_left+m_right)/2, m_right, (m_top+m_down)/2, m_down, m_maxObjects, this, m_level+1, m_maxLevel);
	
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
