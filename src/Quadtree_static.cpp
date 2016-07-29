#include "Quadtree.hpp"

using namespace std;

Quadtree::Quadtree(float _x, float _y, float _width, float _height, int _level, int _maxLevel) :
	x		(_x),
	y		(_y),
	width	(_width),
	height	(_height),
	level	(_level),
	maxLevel(_maxLevel)
{
	if (level == maxLevel) {
		return;
	}

	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;

	NW = new Quadtree(x, y, halfWidth, halfHeight, level+1, maxLevel);
	NE = new Quadtree(x + halfWidth, y, halfWidth, halfHeight, level+1, maxLevel);
	SW = new Quadtree(x, y + halfHeight, halfWidth, halfHeight, level+1, maxLevel);
	SE = new Quadtree(x + halfWidth, y + halfHeight, halfWidth, halfHeight, level+1, maxLevel);
}

Quadtree::~Quadtree() {
	if (level == maxLevel) {
		return;
	}

	delete NW;
	delete NE;
	delete SW;
	delete SE;
}

void Quadtree::AddObject(Object* object) {
	if (level == maxLevel) {
		objects.push_back(object);
		return;
	}

	if (contains(NW, object)) {
		NW->AddObject(object); return;
	} else if (contains(NE, object)) {
		NE->AddObject(object); return;
	} else if (contains(SW, object)) {
		SW->AddObject(object); return;
	} else if (contains(SE, object)) {
		SE->AddObject(object); return;
	}

	if (contains(this, object)) {
		objects.push_back(object);
	}
}

vector<Object*> Quadtree::GetObjectsAt(float _x, float _y) {
	if (level == maxLevel) {
		return objects;
	}
	
	vector<Object*> returnObjects, childReturnObjects;
	if (!objects.empty()) {
		returnObjects = objects;
	}

	const float halfWidth = height * 0.5f;
	const float halfHeight = height * 0.5f;

	if (_x > x + halfWidth && _x < x + width) {
		if (_y > y + halfHeight && _y < y + height) {
			childReturnObjects = SE->GetObjectsAt(_x, _y);
			returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
			return returnObjects;
		} else if (_y > y && _y <= y + halfHeight) {
			childReturnObjects = NE->GetObjectsAt(_x, _y);
			returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
			return returnObjects;
		}
	} else if (_x > x && _x <= x + halfWidth) {
		if (_y > y + halfHeight && _y < y + height) {
			childReturnObjects = SW->GetObjectsAt(_x, _y);
			returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
			return returnObjects;
		} else if (_y > y && _y <= y + halfHeight) {
			childReturnObjects = NW->GetObjectsAt(_x, _y);
			returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
			return returnObjects;
		}
	}

	return returnObjects;
}

void Quadtree::Clear() {
	if (level == maxLevel) {
		objects.clear();
		return;
	} else {
		NW->Clear();
		NE->Clear();
		SW->Clear();
		SE->Clear();
	}

	if (!objects.empty()) {
		objects.clear();
	}
}

void Quadtree::Draw_Rect(int x, int y, int w, int h) {
	GLuint vao, vbo_position, vbo_color;
	glGenVertexArrays(1, &vao);
	
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo_position);
    glGenBuffers(1, &vbo_color);
    
    glBindVertexArray(0);
    
	glUseProgram(GameState::asset.GetShader("shader1.vs").id);
	glBindVertexArray(vao);

	float x1 = (float)(x + 0.5) / GameState::windowSize.x * 2.0 - 1.0;
	float y1 = (float)(y) / GameState::windowSize.y * 2.0 - 1.0;
	float x2 = (float)(x+w) / GameState::windowSize.x * 2.0 - 1.0;
	float y2 = (float)(y+h) / GameState::windowSize.y * 2.0 - 1.0;

	GLfloat positions[] = {
		x1, -y1,
		x1, -y2,
		x2, -y2,
		x2, -y1,
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

void Quadtree::Draw(int x, int y, int w, int h) {
	Draw_Rect(x, y, w, h);
	
	if (level != maxLevel) {
		NW->Draw(NW->x, NW->y, NW->width, NW->height);
		NE->Draw(NE->x, NE->y, NE->width, NE->height);
		SW->Draw(SW->x, SW->y, SW->width, SW->height);
		SE->Draw(SE->x, SE->y, SE->width, SE->height);
	}
}

bool Quadtree::contains(Quadtree* child, Object* object) {
	return !(object->GetPosition().x < child->x ||
             object->GetPosition().y < child->y ||
		     object->GetPosition().x > child->x + child->width  ||
		     object->GetPosition().y > child->y + child->height ||
		     object->GetPosition().x + object->GetSize().x  < child->x ||
		     object->GetPosition().y + object->GetSize().y < child->y ||
		     object->GetPosition().x + object->GetSize().x  > child->x + child->width ||
		     object->GetPosition().y + object->GetSize().y > child->y + child->height);
}
