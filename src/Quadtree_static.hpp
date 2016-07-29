#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <vector>
#include "Object.hpp"
#include "GameState.hpp"

class Quadtree {
	public:
		Quadtree(float x, float y, float width, float height, int level, int maxLevel);
		~Quadtree();

		void AddObject(Object *object);
		std::vector<Object*> GetObjectsAt(float x, float y);
		void Clear();
		
		void Draw_Rect(int x, int y, int w, int h);
		void Draw(int x, int y, int w, int h);

	private:
		float x;
		float y;
		float width;
		float height;
		int	level;
		int maxLevel;
		std::vector<Object*> objects;

		Quadtree* parent;
		Quadtree* NW;
		Quadtree* NE;
		Quadtree* SW;
		Quadtree* SE;

		bool contains(Quadtree *child, Object *object);
};

#endif
