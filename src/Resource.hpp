#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
	public:
		Shader();
		Shader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile = "");
		~Shader();

		void loadShader(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile = "");
		void UseShader();
		void UnuseShader();
		GLuint GetShader();

	private:
		GLuint readShader(std::string shaderFile, GLenum shaderType);

		GLuint m_shader;
};
