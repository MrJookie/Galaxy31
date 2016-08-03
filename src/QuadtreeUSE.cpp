#include "Quadtree.hpp"

Quadtree::Quadtree(int x, int y, int width, int height, int level) {
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	m_level = level;
}

Quadtree::~Quadtree() {
}

void Quadtree::drawRect(int x, int y, int w, int h) {
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
		 1.0, 0.0, 0.0,
		 1.0, 0.0, 0.0,
		 1.0, 0.0, 0.0,
		 1.0, 0.0, 0.0,
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
	this->drawRect(m_x, m_y, m_width, m_height);
	
	if (!this->hasChildren())
		return;
	
	for (auto& child : m_children) {
		child->Draw();	
	}
}

void Quadtree::AddObject(Object* object) {
	if (this->hasChildren() && this->addToChild(object))
		return;

	m_objects.push_back(object);

	// This node is already split, and we can't move any objects down.
	if (this->hasChildren())
		return;

	// Can't split this node, so no point checking number of objects.
	if (m_level == m_maxLevels)
		return;

	// Don't need to split this node yet.
	if (m_objects.size() < m_maxObjects)
		return;

	this->subdivide();

	m_objects.erase(
		std::remove_if(m_objects.begin(), m_objects.end(),
			std::bind(&Quadtree::addToChild, this, std::placeholders::_1)),
		m_objects.end());
}

void Quadtree::subdivide()
{
	double x = m_x;
	double y = m_y;
	double width = m_width / 2.f;
	double height = m_height / 2.f;
	
	m_children[TopLeft]	= std::make_unique<Quadtree>(x, y, width, height, m_level + 1);
	m_children[TopRight] = std::make_unique<Quadtree>(x + width, y, width, height, m_level + 1);
	m_children[BottomLeft] = std::make_unique<Quadtree>(x, y + height, width, height, m_level + 1);
	m_children[BottomRight] = std::make_unique<Quadtree>(x + width, y + height, width, height, m_level + 1);
	
	m_parent = this;
}

void Quadtree::Clear()
{
	m_objects.clear();

	if (!this->hasChildren())
		return;

	for (auto& child : m_children)
	{
		child->Clear();
		child.reset();
	}
}

void Quadtree::GetObjectsAt(int x, int y, int width, int height, std::vector<Object*>& returnObjects) const
{
	if (this->hasChildren())
	{
		auto index = getIndex(x, y, width, height);

		if (index != NotFound)
			m_children[index]->GetObjectsAt(x, y, width, height, returnObjects);
	}

	std::copy(m_objects.begin(), m_objects.end(), std::back_inserter(returnObjects));
}

Quadtree::Quadrant Quadtree::getIndex(int x, int y, int width, int height) const
{
	assert(height > 0.f);
	assert(width > 0.f);

	double verticalMidpoint = m_x + m_width / 2;
	double horizontalMidpoint = m_y + m_height / 2;

	// Can the object "completely" fit within this quadrant?
	bool top = (y + height < horizontalMidpoint);
	bool bottom = (y > horizontalMidpoint);
	bool left = (x + width < verticalMidpoint);
	bool right = (x > verticalMidpoint);

	if (top && left)
		return TopLeft;

	if (top && right)
		return TopRight;

	if (bottom && left)
		return BottomLeft;

	if (bottom && right)
		return BottomRight;
	
	return NotFound;
}


bool Quadtree::addToChild(Object* object) const
{
	assert(this->hasChildren());
	
	double x = object->GetPosition().x - object->GetSize().x / 2;
	double y = object->GetPosition().y - object->GetSize().y / 2;
	double width = object->GetSize().x;
	double height = object->GetSize().y;

	auto index = getIndex(x, y, width, height);

	if(index == NotFound)
		return false;

	m_children[index]->AddObject(object);

	return true;
}

bool Quadtree::hasChildren() const
{
	return(m_children[0] != nullptr);
}
