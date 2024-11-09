#pragma once
#include <string>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <glm.hpp>

class Shader {
public:
	unsigned int ID{0};

	Shader(const char* vertex_source, const char* fragment_source);
	void use_program();

	void set_bool(const std::string& name, bool value);
	void set_int(const std::string& name, int value);
	void set_float(const std::string& name, float value);
	void set_mat4(const std::string& name, glm::mat4 value);
};