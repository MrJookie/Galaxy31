#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include "Object.hpp"
#include "GameState.hpp"

class Quadtree {
	public:
		Quadtree(int left, int right, int top, int down, unsigned int maxObjects = 2, Quadtree* parent = nullptr, int level = 1, int maxLevel = 5);
		~Quadtree();
				
		void AddObject(Object* object);
		void QueryRectangle(int x, int y, int w, int h, std::vector<Object*>& returnObjects);
		void GetObjectsInNode(std::vector<Object*>& returnObjects);
		void Draw();
		void DrawRect(int x, int y, int w, int h, glm::vec4 color);
		void Clear();
		void Resize();

	private:
		int m_left;
		int m_right;
		int m_top;
		int m_down;
		bool m_isLeaf;
		unsigned int m_maxObjects;
		std::vector<Object*> m_objects;
		
		std::vector<std::unique_ptr<Quadtree>> m_children;
		Quadtree* m_parent;
		
		void createLeaves();
		void moveObjectsToLeaves();
		
		bool contains(Object* object);
		bool intersects(int x, int y, int w, int h);
		
		int m_level;
		int m_maxLevel;
};

#endif
