#include "Quadtree.hpp"

Quadtree::Quadtree() :
left( 0 ),
right( 0 ),
top( 0 ),
down( 0 ),
nodes( 0 ),
isLeaf( true )
{
}

Quadtree::Quadtree( double _left, double _right, double _top, double _down, unsigned int _numObjectsToGrow ) :
left( _left ),
right( _right ),
top( _top ),
down( _down ),
numObjectsToGrow( _numObjectsToGrow ),
nodes( 0 ),
isLeaf( true )
{
}

Quadtree::~Quadtree()
{
	if ( !isLeaf )
		delete [] nodes;
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
	Draw_Rect(left, top, right-left, down-top);

	if ( !isLeaf ) {
		for ( int n = 0; n < NodeCount; ++n ) {
			nodes[n].Draw();
		}
	}
}

void Quadtree::AddObject( Object *object )
{
	if ( isLeaf ) {
		objects.push_back( object );
		bool reachedMaxObjects = ( objects.size() == numObjectsToGrow );
		if ( reachedMaxObjects ) {
			createLeaves();
			moveObjectsToLeaves();
			isLeaf = false;
		}
		return;
	}

	for ( int n = 0; n < NodeCount; ++n ) {
		if ( nodes[n].contains( object ) ) {
			nodes[n].AddObject( object );
			return;
		}
	}

	objects.push_back( object );
}

void Quadtree::Clear()
{
	objects.clear();

	if ( !isLeaf ) {
		for ( int n = 0; n < NodeCount; ++n ) {
			nodes[n].Clear();
		}
	}
}

vector<Object*> Quadtree::GetObjectsAt( double x, double y )
{
	if ( isLeaf ) {
		return objects;
	}

	vector<Object*> returnedObjects;
	vector<Object*> childObjects;

	if ( !objects.empty() )
		returnedObjects.insert( returnedObjects.end(), objects.begin(), objects.end() );

	for ( int n = 0; n < NodeCount; ++n ) {
		if ( nodes[n].contains( x, y ) ) {
			childObjects = nodes[n].GetObjectsAt( x, y );
			returnedObjects.insert( returnedObjects.end(), childObjects.begin(), childObjects.end() );
			break;
		}
	}
	
	return returnedObjects;
}

bool Quadtree::contains( Object *object )
{
	return 	(object->GetPosition().x > left &&
		(object->GetPosition().x + object->GetSize().x) < right &&
		object->GetPosition().y > top &&
		(object->GetPosition().y + object->GetSize().y) < down);
}

bool Quadtree::contains( double x, double y )
{
	return 	( x >= left && x <= right ) &&
		( y >= top && y <= down );
}

void Quadtree::createLeaves()
{
	nodes = new Quadtree[4];
	nodes[NW] = Quadtree( left, (left+right)/2, top, (top+down)/2, numObjectsToGrow );
	nodes[NE] = Quadtree( (left+right)/2, right, top, (top+down)/2, numObjectsToGrow );
	nodes[SW] = Quadtree( left, (left+right)/2, (top+down)/2, down, numObjectsToGrow );
	nodes[SE] = Quadtree( (left+right)/2, right, (top+down)/2, down, numObjectsToGrow );
}

void Quadtree::moveObjectsToLeaves()
{
	for ( int n = 0; n < NodeCount; ++n ) {
		for ( unsigned int m = 0; m < objects.size(); ++m ) {
			if ( nodes[n].contains( objects[m] ) ) {
				nodes[n].AddObject( objects[m] );
				objects.erase( objects.begin() + m );
				--m;
			}
		}
	}
}
