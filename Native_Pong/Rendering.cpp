#include <glad/glad.h> //helps us get the function pointers of OpenGL
#include <glfw/glfw3.h> //manage the windows and opengl context, receive inputs and events
#include <glm.hpp> //math function utils
#include <iostream>
#include "Shader.h"
#include "ImGUI_Wrapper.h"
#include <Windows.h>
#include "Rectangle.cpp"
#include "Circle.cpp"
/*
everytime we resize the glfw window, we should also resize the opengl viewport. this function lets us do that.
it gets called everytime we rezie the glfw window to also resize
*/
class Render {
public:
	GLFWwindow* window;
	imgui_wrapper* wrapper;
	Shader* shader;

	float last_time{ 0 };

	Render() {
		window = NULL;
		shader = nullptr;
	}

	void init(int width, int height, float* rgba, void (*key_callback)(GLFWwindow* window, int key, int scancode, int action, int mods)) {
		std::cout << "init called" << '\n';
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, "Native Pong", NULL, NULL);
		if (window == NULL) {
			std::cout << "ERROR CREATING THE GAME WINDOW";
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);

		//init glad
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			//if init fails
			std::cout << "Failed to Initialize GLAD" << std::endl;
		}

		glViewport(0, 0, width, height); //specifies the window rectangle that is going to be used to actually render in opengl. in this case its going to be the whole window
		
		//glEnable(GL_DEPTH_TEST);

		glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]); //we set the color of the glclear command to black
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//set callbacks
		glfwSetKeyCallback(window, key_callback);
		//init IMGUI
		wrapper = new imgui_wrapper();
		wrapper->init(window);
	}

	void set_shader(const char* vertex_path, const char* fragment_path) {
		shader = new Shader(vertex_path, fragment_path);
	}

	void create_VAO_element_buffer(float* pvertices, unsigned int* pindices, int psize_vertices, int psize_indices, unsigned int& ret_VAO, unsigned int& ret_VBO) {
		//CREATE THE BUFFERS/OBJECTS
		//remember that just binding the VAO binds the following code calls to it.
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		unsigned int EBO;	
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		//BIND DATA TO THE BUFFERS
		glBufferData(GL_ARRAY_BUFFER, psize_vertices, pvertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, psize_indices, pindices, GL_STATIC_DRAW);


		//DESCRIBE DATA PASSED
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(NULL); //unbind the VAO so further calls are not added
		ret_VAO = VAO;
		ret_VBO = VBO;
	}

	void create_VAO(float* pvertices, int psize_vertices, unsigned int& ret_VAO, unsigned int& ret_VBO) {
		unsigned int VAO;
		
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//BIND DATA TO THE BUFFERS
		glBufferData(GL_ARRAY_BUFFER, psize_vertices, pvertices, GL_STATIC_DRAW);

		//DESCRIBE DATA PASSED
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(NULL); //unbind the VAO so further calls are not added

		ret_VAO = VAO;
		ret_VBO = VBO;
	}

	int get_buffer_size(){
		int size{};
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		return size;
	}

	void change_VBO_Vertices(float* pvertices, int psize_vertices, unsigned int pVAO, unsigned int pVBO) {
		glBindVertexArray(pVAO);
		glBindBuffer(GL_ARRAY_BUFFER, pVBO);
		
		glBufferSubData(GL_ARRAY_BUFFER, 0, psize_vertices, pvertices);
		GLenum err;
		err = glGetError();
		if (err != 0) {
			std::cout << "OPENGL ERROR DETECTED: ENUM IS " << err << '\n';
		}
	}

	float start_rendering(bool show_imgui, std::vector<rectangle*> fig_list, Circle pball) {
		glfwPollEvents();

		//imgui rendering setup
		wrapper->start_render();
		if (show_imgui == true){
			wrapper->create_imgui_window();
			//print ball center pos
			std::vector<std::string> value_titles{"ball center x: %.3f", "ball center y: %.3f"};
			std::vector<float> values{ pball.center[0], pball.center[1]};
			wrapper->create_custom_imgui_window("ball position", value_titles, values);
			
			value_titles = { "border center x: %.3f", "border center y: %.3f", "border width: %.3f", "border height: %.3f" };
			values = { fig_list[0]->center[0], fig_list[0]->center[1], fig_list[0]->width, fig_list[0]->height};
			wrapper->create_custom_imgui_window("top_border_position", value_titles, values);
		}
		//ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		shader->use_program();

		//render rectangles
		for (int i = 0; i < fig_list.size(); ++i) {
			glBindVertexArray((*fig_list[i]).VAO);
			glDrawElements(GL_TRIANGLES, (*(fig_list[i])).indices.size(), GL_UNSIGNED_INT, 0);
		}

		//render the ball
		glBindVertexArray(pball.VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, pball.vertices.size() / 3);

		//CREATE DEBUGGING LINES TO MEASURE WHERE HALF OF A QUADRANT IS
		/*
		unsigned int line_VAO;
		unsigned int line_VBO;
		std::vector<float> line_verts = { -1.0f, 0.5f, 0.0f,
										   1.0f, 0.5f, 0.0f };
		create_VAO(line_verts.data(), sizeof(float) * line_verts.size(), line_VAO, line_VBO);
		glBindVertexArray(line_VAO);
		glDrawArrays(GL_LINES, 0, 2);
		*/

		GLenum err;
		err = glGetError();
		if (err != 0) {
			std::cout << "OPENGL ERROR DETECTED: ENUM IS " << err << '\n';
		}
		wrapper->render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		
		//calculate deltaTime 
		float current_time = glfwGetTime();
		float deltaTime = current_time - last_time;
		last_time = current_time;

		return deltaTime;
	}
};
