#include "Shader.h"
#include <glad/glad.h>

Shader::Shader(const char* vertex_source, const char* fragment_source)
{
	bool succesfull_op{ true };
	std::ifstream vShaderFile, fShaderFile; //stream from the vertex shader file
	std::string vertexcode, fragmentcode; //vertex and fragment shader shader source code
	std::stringstream vShaderStream, fShaderStream; //objects to operate on strings
	const char* vertex_c_str;
	const char* fragment_c_str;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		vShaderFile.open(vertex_source);
		vShaderStream << vShaderFile.rdbuf();
		vertexcode = vShaderStream.str();
		vertex_c_str = vertexcode.c_str();
	}
	catch (std::ifstream::failure failure) {
		std::cout << "ERROR READING VERTEX SHADER FILE" << '\n';
		succesfull_op = false;
	}

	try {
		fShaderFile.open(fragment_source);
		fShaderStream << fShaderFile.rdbuf();
		fragmentcode = fShaderStream.str();
		fragment_c_str = fragmentcode.c_str();
	}
	catch (std::ifstream::failure failure) {
		std::cout << "ERROR READING FRAGMENT SHADER FILE" << '\n';
		succesfull_op = false;
	}

	unsigned int vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, &vertex_c_str, NULL);
	glShaderSource(fragmentShader, 1, &fragment_c_str, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	//debug variables
	int success;
	char infolog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
		std::cout << "ERROR: VERTEX SHADER NOT COMPILED SUCCESFULLY" << '\n' << infolog;
		succesfull_op = false;
	}

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
		std::cout << "ERROR: FRAGMENT SHADER NOT COMPILED SUCCESFULLY" << '\n' << infolog;
		succesfull_op = false;
	}

	ID = glCreateProgram();

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infolog);
		std::cout << "ERROR: SHADER PROGRAM LINKING FAILED" << '\n' << infolog;
		succesfull_op = false;
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (succesfull_op) {
		std::cout << "SHADER PROGRAM COMPILED AND LINKED SUCCESFULLY. ID IS " << ID << '\n';
	}
}

void Shader::use_program()
{
	glUseProgram(ID);
}

void Shader::set_bool(const std::string& name, bool value)
{
	int uniform_location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uniform_location, value);
}

void Shader::set_int(const std::string& name, int value)
{
	int uniform_location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uniform_location, value);
}

void Shader::set_float(const std::string& name, float value)
{
	int uniform_location = glGetUniformLocation(ID, name.c_str());
	glUniform1f(uniform_location, value);
}

void Shader::set_mat4(const std::string& name, glm::mat4 value) {
	int uniform_location = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_vec3(const std::string& name, glm::vec3 value)
{
	int uniform_location = glGetUniformLocation(ID, name.c_str());
	glUniform3f(uniform_location, value.x, value.y, value.z);
}
