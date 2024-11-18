#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <vector>
#pragma once
class imgui_wrapper {
	public:
		void init(GLFWwindow* window);
		void shutdown();
		void create_imgui_window();
		void float_create_custom_imgui_window(std::string title, std::vector<std::string> value_titles, std::vector<float> values);
		void int_create_custom_imgui_window(std::string title, std::vector<std::string> value_titles, std::vector<int> values);
		void start_render();
		void render();
};