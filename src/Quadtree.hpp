#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <vector>
#include "Object.hpp"
using std::vector;

class Quadtree {

enum Node {
	NW = 0,
	NE,
	SW,
	SE,
	NodeCount
};

public:
					Quadtree();

					Quadtree( double left, double right, double top, double down, unsigned int numObjectsToGrow = 2 );

					~Quadtree();

	void				AddObject( Object *object );

	void				Clear();

	vector<Object*>			GetObjectsAt( double x, double y );

private:
	double				left;

	double				right;

	double				top;

	double				down;

	unsigned int			numObjectsToGrow;

	vector<Object*>			objects;

	Quadtree * 			nodes;

	bool				isLeaf;
	
	bool				contains( Object *object );
	
	bool				contains( double x, double y );

	void				createLeaves();

	void				moveObjectsToLeaves();
	
};

#endif
