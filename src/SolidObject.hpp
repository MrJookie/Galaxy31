#ifndef SOLID_OBJECT_HPP
#define SOLID_OBJECT_HPP

//opengl
#include <GL/glew.h>

//stl
#include <unordered_map>
#include <vector>

//glm
#include <glm/glm.hpp>

#include "Object.hpp"

class SolidObject : public Object {
	public:
		SolidObject(glm::vec2 size = {0,0}, glm::vec2 position = {0,0}, float rotation = 0, glm::vec2 speed = {0,0}) : Object(size,position,rotation,speed) {}

		~SolidObject() {}
		
		virtual void Draw() {};
		
		void UpdateHullVertices(std::vector<glm::vec2> hullVertices);
		void RenderCollisionHull();
		std::vector<glm::vec2> GetCollisionHull();
		glm::vec4 CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
		bool DoObjectsIntersect(SolidObject* obj);
		bool DoLinesIntersect(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);
		std::vector<glm::vec2> m_hullVertices;
		
	private:

};

#endif
