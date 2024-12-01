#include <glad/glad.h> //helps us get the function pointers of OpenGL
#include <glfw/glfw3.h> //manage the windows and opengl context, receive inputs and events
#include <glm.hpp> //math function utils
#include <iostream>
#include "Shader.h"
#include "ImGUI_Wrapper.h"
#include <Windows.h>
#include "Rectangle.cpp"
#include "Circle.cpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Character.cpp"
#include <map>
#include <string>

/*
everytime we resize the glfw window, we should also resize the opengl viewport. this function lets us do that.
it gets called everytime we rezie the glfw window to also resize
*/
class Render {
public:
	GLFWwindow* window;
	imgui_wrapper* wrapper;

	Shader* shader;
	Shader* text_shader;
	
	unsigned int text_VAO;
	unsigned int text_VBO;
	FT_Library freetype;
	FT_Face typeface;
	
	std::map<char, Character> characters; //well map every char we want to use into a Character struct we create on app init.
	float last_time{ 0 };

	Render() {
		window = NULL;
		//shader = nullptr;
		shader = NULL;
		text_shader = NULL;
	}

	void get_gl_error() {
		GLenum err;
		err = glGetError();
		if (err != 0) {
			std::cout << "OPENGL ERROR DETECTED: ENUM IS " << err << '\n';
		}
	}
	Character* create_character_struct(unsigned char c) {
		if (FT_Load_Char(typeface, c, FT_LOAD_RENDER)) {
			std::cout << "Failed to load Glyph: " << c << '\n';
			return NULL;
		}
		//generate glyph texture
		unsigned int texture;
		glGenTextures(1, &texture); //generate the opengl texture object
		glBindTexture(GL_TEXTURE_2D, texture); //bind the texture to be currently active
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED, //FreeType texture glyph info is stored only in the red component
			typeface->glyph->bitmap.width,
			typeface->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			typeface->glyph->bitmap.buffer
		); //Generate the 2d texture image object onto the currenly bound texture object
		//set texture options and configurations
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//create character struct
		Character* character = new Character(
			texture,
			glm::ivec2(typeface->glyph->bitmap.width, typeface->glyph->bitmap.rows),
			glm::ivec2(typeface->glyph->bitmap_left, typeface->glyph->bitmap_top),
			typeface->glyph->advance.x
		);
		return character;
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
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]); //we set the color of the glclear command to black
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//set callbacks
		glfwSetKeyCallback(window, key_callback);

		//init IMGUI
		wrapper = new imgui_wrapper();
		wrapper->init(window);

		//init FreeType
		if (FT_Init_FreeType(&freetype)) {
			std::cout << "COULD NOT INIT FREETYPE LIBRARY" << '\n';
		}

		if (FT_New_Face(freetype, "CONSOLA.ttf", 0, &typeface)) {
			std::cout << "COULD NOT LOAD TYPEFACE" << '\n';
		}

		//set the pixel font size, setting the height to 48 and the width to 0 (this means it automatically calculates the width based on the height)
		FT_Set_Pixel_Sizes(typeface, 0, 48);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
		for (unsigned char c = 48; c <= 53; ++c) { //only load ascii chars for numbers 0 to 5 (only used to keep scores)
			Character* char_struct = create_character_struct(c);
			
			std::pair<char, Character> pair_to_add {c, *char_struct};
			characters.insert(pair_to_add);
		}
		//load characters we are going to use
		unsigned char letters[15] = { 80, 76, 65, 89, 69, 82, 87, 79, 78, 73, 32, 83, 67, 84, 71};
		for (int i = 0; i < 15; ++i) {
			Character* char_struct = create_character_struct(letters[i]);

			std::pair<char, Character> pair_to_add{ letters[i], *char_struct};
			characters.insert(pair_to_add);
		}
		//create the text rendering VAO
		create_text_rendering_VAO(text_VAO, text_VBO);

		//create the text rendering shader
		text_shader = new Shader("text_vertex_shader.txt", "text_fragment_shader.txt");

		//free freetype resources
		FT_Done_Face(typeface);
		FT_Done_FreeType(freetype);
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
		get_gl_error();
	}

	void create_text_rendering_VAO(unsigned int& ret_VAO, unsigned int& ret_VBO) {
		glGenVertexArrays(1, &ret_VAO);
		glBindVertexArray(ret_VAO);

		glGenBuffers(1, &ret_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, ret_VBO);

		//bind data to the buffer
		glBufferData(GL_ARRAY_BUFFER, 
			sizeof(float) * 6 * 4, //every text quad is made up of 6 vertices (2 triangles) of 4 floats each(2 for vertex coordinates, 2 for texture coordinates)
			NULL, //no data at this time, it will be added later when we want to render text
			GL_DYNAMIC_DRAW); //the buffers data will be changed often

		//describe data passed
		glVertexAttribPointer(0, //starting index of the vertex attribute, 0 since there will only be one for this VAO
			4, //number of components of every vertex attribute
			GL_FLOAT, //datatype of every component
			GL_FALSE, //data is not normalized
			4 * sizeof(float), //byte offset between vertex attributes, since every vertex attribute is made up of 4 floats, we give 4 * sizeof(float)
			0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}
	
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
		//activate corresponding render state
		text_shader->use_program();
		text_shader->set_vec3("textColor", color);
		text_shader->set_mat4("projection", glm::ortho(0.0f, 800.0f, 0.0f, 600.0f));
		glActiveTexture(GL_TEXTURE0); //activate a given texture unit, this has a maximum given by GL_MAX_TEXTURE_UNITS
		get_gl_error();
		glBindVertexArray(text_VAO);
		get_gl_error();

		//iterate through all text characters
		//for every character we generate a quad and 
		for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
			Character ch = characters[*c]; //get the character struct of the current char

			float xpos = x + ch.bearing.x * scale;
			float ypos = y - (ch.size.y - ch.bearing.y) * scale;

			float w = ch.size.x * scale;
			float h = ch.size.y * scale;
			//update VBO for each character
			//6 vertices of 4 components(2 for the position, )
			float vertices[6][4] = {
				//first triangle
				{xpos, ypos + h,	 0.0f, 0.0f},
				{xpos, ypos,		 0.0f, 1.0f},
				{xpos + w, ypos,	 1.0f, 1.0f},
				//second triangle
				{xpos, ypos + h,	 0.0f, 0.0f},
				{xpos + w, ypos,	 1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}
			};
			glBindTexture(GL_TEXTURE_2D, ch.textureID); //bind the char texture ID that we generated on init
			glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); //set the VBO data
			glBindBuffer(GL_ARRAY_BUFFER, NULL); //unbind the text rendering vbo
			glDrawArrays(GL_TRIANGLES, 0, 6); //render the quad
			x += (ch.advance >> 6) * scale;//advance in the x direction for the next glyph. bitshift by 6 to get value in pixels(2^6 = 64)
		}
		glBindVertexArray(NULL); //unbind the VAO
		glBindTexture(GL_TEXTURE_2D, NULL); //unbind the texture
	}

	float start_rendering(bool show_imgui, std::vector<rectangle*> fig_list, Circle pball, int* scores, float x, float y, int winner) {
		glfwPollEvents();

		//imgui rendering setup
		wrapper->start_render();
		if (show_imgui == true){
			wrapper->create_imgui_window();
			//print ball center pos
			std::vector<std::string> value_titles{"ball center x: %.3f", "ball center y: %.3f"};
			std::vector<float> values{ pball.center[0], pball.center[1]};
			wrapper->float_create_custom_imgui_window("ball position", value_titles, values);
			
			value_titles = { "player: %d", "AI: %d" };
			std::vector<int> score_values = { scores[0], scores[1] };
			wrapper->int_create_custom_imgui_window("scores", value_titles, score_values);

			value_titles = { "text_pos x: %.3f" , "text_pos y: %.3f" };
			std::vector<float> pos_values = {x, y};
			wrapper->float_create_custom_imgui_window("text position", value_titles, pos_values);
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

		//CREATE DEBUGGING LINES TO MEASURE WHERE HALF the screen
		
		unsigned int line_VAO;
		unsigned int line_VBO;
		std::vector<float> line_verts = {  0.0f, 1.0f, 0.0f,
										   0.0f, -1.0f, 0.0f };
		create_VAO(line_verts.data(), sizeof(float) * line_verts.size(), line_VAO, line_VBO);
		glBindVertexArray(line_VAO);
		glDrawArrays(GL_LINES, 0, 2);
		

		//draw the scores
		//RenderText(std::to_string(scores[0]), 368.0f, 553.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		//RenderText(std::to_string(scores[0]), x, y, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		//RenderText(std::to_string(scores[1]), 406.0f, 553.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		//RenderText(std::to_string(scores[1]), x, y, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		RenderText(std::to_string(scores[0]), 350.0f, 553.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		RenderText(std::to_string(scores[1]), 420.0f, 553.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		//if winner param is 0, no one has yet, if its 1, player won, if its 2, ai won.
		if (winner == 1) { //player won
			RenderText("PLAYER WON", 270.0f, 420.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			//RenderText("PLAYER WON", x, y, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			RenderText("PRESS SPACE TO PLAY AGAIN", 63.0f, 31.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		else if (winner == 2) {
			RenderText("AI WON", 335.0f, 420.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			RenderText("PRESS SPACE TO PLAY AGAIN", 63.0f, 31.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		}

		get_gl_error();

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
