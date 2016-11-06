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
#include "Sprite.hpp"

class Quadtree;

class SolidObject : public Object {
	public:
		SolidObject(glm::dvec2 size = {0,0}, glm::dvec2 position = {0,0}, float rotation = 0, glm::dvec2 speed = {0,0}) : Object(size,position,rotation,speed) {}

		~SolidObject() {}
		
		virtual void Draw() {};
		virtual Sprite* GetSprite() = 0;
		
		void UpdateHullVertices(std::vector<glm::vec2> hullVertices);
		void RenderCollisionHull();
		std::vector<glm::vec2> GetCollisionHull();
		glm::vec4 CollisionHullColor = glm::vec4(1.0, 0.0, 1.0, 1.0);
		
		void RenderProjectileRay(std::vector<glm::vec2> startEndPoints);
		void UpdateProjectileRay(std::vector<glm::vec2> rayVertices);
		std::vector<glm::vec2> GetProjectileRay();
		bool CollidesProjectileRay(SolidObject* obj);
		glm::vec4 CollidesProjectileRayColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
		
		bool DoObjectsIntersect(SolidObject* obj);
		bool DoLinesIntersect(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);
		bool Collides(SolidObject* obj);
		
		std::vector<glm::vec2> m_hullVertices;
		std::vector<glm::vec2> m_projectileRayVertices;
		Quadtree* NodePtr;
		
	private:

};

#endif
