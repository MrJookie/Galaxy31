#include <SDL2/SDL.h>

#include <memory>

#include <vector>
#include <iostream>

#include <algorithm>

#include "Object.hpp"
#include "GameState.hpp"

class Quadtree {
	enum Quadrant
	{
		NotFound = -1,
		TopLeft, 
		TopRight,
		BottomLeft, 
		BottomRight,
	};
	
	public:
		Quadtree(int x, int y, int w, int h, int level);
		~Quadtree();
	
		void Draw();
		void AddObject(Object* object);
		void GetObjectsAt(int x, int y, int width, int height, std::vector<Object*>& returnObjects) const;
		void Clear();
		
	private:
		void drawRect(int x, int y, int w, int h);
		Quadtree::Quadrant getIndex(int x, int y, int width, int height) const;
		bool addToChild(Object* object) const;
		bool hasChildren() const;
		void subdivide();
		
		std::vector<Object*> m_objects;
		std::array<std::unique_ptr<Quadtree>, 4> m_children;
	
		Quadtree* m_parent;
		
		int m_x;
		int m_y;
		int m_width;
		int m_height;
		int m_maxObjects = 6;
		int m_maxLevels = 20;
		int m_level;
};
