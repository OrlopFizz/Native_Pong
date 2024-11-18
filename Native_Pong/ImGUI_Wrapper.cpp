#include "ImGui_Wrapper.h"
#include <string>
#include <vector>

void imgui_wrapper::init(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void imgui_wrapper::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
} 
void imgui_wrapper::create_imgui_window() {
	ImGui::Begin("First Window");
	ImGui::Text("time to render last frame: %.1f", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("time to render last frame2: %.1f", 1000.0f * ImGui::GetIO().DeltaTime);
	ImGui::Text("current framerate: %.3f", ImGui::GetIO().Framerate);
	ImGui::End();
}

//value_titles aand values must have the same size
void imgui_wrapper::float_create_custom_imgui_window(std::string title, std::vector<std::string> value_titles, std::vector<float> values) {
	ImGui::Begin(title.c_str());
	for (int i = 0; i < value_titles.size(); ++i) {
		ImGui::Text(value_titles[i].c_str(), values[i]);
	}
	ImGui::End();
}

void imgui_wrapper::int_create_custom_imgui_window(std::string title, std::vector<std::string> value_titles, std::vector<int> values) {
	ImGui::Begin(title.c_str());
	for (int i = 0; i < value_titles.size(); ++i) {
		ImGui::Text(value_titles[i].c_str(), values[i]);
	}
	ImGui::End();
}

void imgui_wrapper::start_render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void imgui_wrapper::render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}