#include <iostream>
#include <map>
#include <algorithm>
#include "Rectangle.cpp"
#include "Circle.cpp"
#include "Rendering.cpp"
#include <algorithm>
#include "collision_funcs.h"

static bool show_imgui{false};
static float world_space_x = 10000.0f; //WORLD COORDINATES, IT ONLY SUPPORTS SYMMETRICAL WORLDS FOR NOW
static float world_space_y = 6000.0f;
static int screen_size_width = 1000;
static int screen_size_height = 600;
//aspect ratio de 1.6666666, es decir, por cada unidad de altura(y) el valor de x debe multiplicarse uno por 1.66666666
static float aspect_ratio = ((float)screen_size_width / (float)screen_size_height);
static float dt = 0.0f;
rectangle* player_bumper;
Render* render = new Render();

float x{ 403.0f };
float y{ 553.0 };
bool w_key_state{ false };
bool s_key_state{ false };
int scores[2] {0, 0};

void framebuffer_size_callback(GLFWwindow* window, int pwidth, int pheight) {
	glViewport(0, 0, pwidth, pheight); //resize the viewport with the given width and height
	screen_size_width = pwidth;
	screen_size_height = pheight;
	aspect_ratio = (float)(screen_size_width / screen_size_height);
	std::cout << "NEW WIDTH: " << screen_size_width << '\n';
	std::cout << "NEW HEIGHT: " << screen_size_height << '\n';
}


std::vector<float> world_space_to_render_space(std::vector<float> pvertices) {
	std::vector<float> ret_verts;
	for (int i = 0; i < pvertices.size(); i+=3) {
		ret_verts.push_back(pvertices[i] / world_space_x);
		ret_verts.push_back(pvertices[i+1] / world_space_y);
		ret_verts.push_back(0.0f);
	}
	return ret_verts;
}

std::vector<float> apply_aspect_ratio_to_vertices(std::vector<float> pvertices) {
	//apply the aspect ratio to the y value of the vertices
	for (int i = 1; i < pvertices.size(); i = i + 3) {
		pvertices[i] = std::clamp(pvertices[i] * aspect_ratio, -1.0f, 1.0f);
	}
	return pvertices;
}

void process_input(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true) ;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		std::cout << "key press" << '\n';
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		std::cout << "key release" << '\n';
	}
	if (key == GLFW_KEY_J && action == GLFW_REPEAT) {
		std::cout << "key repeat" << '\n';
	}
	if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
		show_imgui = !show_imgui;
	}
	if (key == GLFW_KEY_UP) {
		y += 1.0f;
	}
	if (key == GLFW_KEY_DOWN) {
		y -= 1.0f;
	}
	if (key == GLFW_KEY_LEFT) {
		x -= 1.0f;
	}
	if (key == GLFW_KEY_RIGHT) {
		x += 1.0f;
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		w_key_state = true;
	}
	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		w_key_state = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		s_key_state = true;
	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		s_key_state = false;
	}

	if (key == GLFW_KEY_L && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		std::cout << "key pressed" << '\n';
	} 
}

rectangle* get_collision_with_bumpers(std::array<rectangle*, 2> bumper_list, Circle* cir) {
	for (rectangle* rec : bumper_list) {
		//get every point 
		std::array<float, 2> bottom_left = { (*rec).vertices[0],(*rec).vertices[1] };
		std::array<float, 2> bottom_right = { (*rec).vertices[3], (*rec).vertices[4] };
		std::array<float, 2> top_right = { (*rec).vertices[6], (*rec).vertices[7] };
		std::array<float, 2> top_left = { (*rec).vertices[9], (*rec).vertices[10] };

		//get the sides of the rectangle
		//get an array of every side in the following order: bottom_side, right_side, top_side, left_side
		std::array<std::array<float, 2>, 2> bottom_side = { bottom_left, bottom_right };
		std::array<std::array<float, 2>, 2> right_side = { bottom_right, top_right };
		std::array<std::array<float, 2>, 2> top_side = { top_right, top_left };
		std::array<std::array<float, 2>, 2> left_side = { top_left, bottom_left };
		std::array<std::array<std::array<float, 2>, 2>, 4> rec_sides = {bottom_side, right_side, top_side, left_side};
		for (std::array<std::array<float, 2>, 2> side : rec_sides) {
			//check for collision between the side and the circle
			bool collision = circle_collision_with_line(side[0], side[1], cir);
			if (collision) { return rec; }
		}
	}
	return NULL;
}

rectangle* get_collision_with_borders(std::array<rectangle*, 2> border_list, Circle* cir, std::map<std::array<float, 2>, std::array<float, 2>> rect_projections) {
	for (rectangle* rec : border_list) {
		std::array<float, 2> bottom_left = { (*rec).vertices[0],(*rec).vertices[1] };
		std::array<float, 2> bottom_right = { (*rec).vertices[3], (*rec).vertices[4] };
		std::array<float, 2> top_right = { (*rec).vertices[6], (*rec).vertices[7] };
		std::array<float, 2> top_left = { (*rec).vertices[9], (*rec).vertices[10] };

		std::array<std::array<float, 2>, 4> border_points = { bottom_left, bottom_right, top_left, top_right };

		//get the axis to project into
		std::array<float, 2> projection_axis = rect_projections[rec->center];
		//project point to an axis
		float projected_center = project_point_to_axis(cir->center, projection_axis);
		float ball_min = projected_center - cir->radious;
		float ball_max = projected_center + cir->radious;
		//project the border points into the projection axis
		std::vector<float> projections_1 {};
		for (std::array<float, 2> border : border_points) {
			float projected_vector_1 = project_point_to_axis(border, projection_axis);
			projections_1.push_back(projected_vector_1);
		}
		std::sort(projections_1.begin(), projections_1.end());
		float border_min = projections_1[0];
		float border_max = projections_1[projections_1.size()-1];
		if (overlap(border_min, border_max, ball_min, ball_max) != 0.0f) {
			//there was overlap, so there was a collision
			return rec;
		}
	}
	return NULL;
}

void calculate_ai_movement(rectangle* ai_bumper, Circle* cir) {
	if (ai_bumper->center[1] < cir->center[1]) { //the ball is up from the bumper, move that way
		ai_bumper->move_rectangle(0.0f, 6000.0f * dt);
		std::vector<float> render_verts = world_space_to_render_space(ai_bumper->vertices);
		render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), (*ai_bumper).VAO, (*ai_bumper).VBO);
	}
	else if (ai_bumper->center[1] > cir->center[1]) { //the ball is down from the bumper, move that way
		ai_bumper->move_rectangle(0.0f, -6000.0f * dt);
		std::vector<float> render_verts = world_space_to_render_space(ai_bumper->vertices);
		render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), (*ai_bumper).VAO, (*ai_bumper).VBO);
	}
}

int main() {
	std::cout << "hello world" << '\n';

	rectangle top_border, left_border, bottom_border, right_border;

	top_border.create_rectangle({0.0f, world_space_y - 100}, 2 * world_space_x, 200.0f);
	left_border.create_rectangle({-(world_space_x - 100), 0.0f}, 200.0f, 2 * world_space_y);
	bottom_border.create_rectangle({0.0f, -(world_space_y - 100)}, 2 * world_space_x, 200.0f);
	right_border.create_rectangle({ world_space_x - 100, 0.0f}, 200.0f, 2 * world_space_y);

	std::map<std::array<float, 2>, std::array<float, 2>> rect_projections{ {top_border.center, {0.0f, 1.0f}}, {bottom_border.center, {0.0f, 1.0f}}, {left_border.center, {1.0f, 0.0f}}, {right_border.center, {1.0f, 0.0f}} };

	rectangle right_bumper, left_bumper;
	left_bumper.create_rectangle({ -7500.0f, 0.0f }, 200.0f, 2000.0f);
	right_bumper.create_rectangle({7500.0f, 0.0f}, 200.0f, 2000.0f);

	player_bumper = &left_bumper;

	//std::vector<rectangle*> border_recs{ &top_border, &left_border, &bottom_border, &right_border, &right_bumper, &left_bumper };
	std::vector<rectangle*> border_recs{ &top_border, &bottom_border, &right_bumper, &left_bumper};
	std::array<rectangle*, 2> bumpers{ &right_bumper, &left_bumper };
	Circle cir;
	cir.create_circle({0.0f, 0.0f }, 250.0f);

	//create render object
	float rgba[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	(*render).init(screen_size_width, screen_size_height, rgba, process_input);
	
	//PASSING TO THE RENDER ENGINE
	//SET VAO
	//transform world coordinates to render coordinates
	
	for (rectangle* rec : border_recs) {
		std::vector<float> render_verts = world_space_to_render_space((*rec).vertices);
		//std::vector<float> ar_verts = apply_aspect_ratio_to_vertices(render_verts);
		render->create_VAO_element_buffer(render_verts.data(), (*rec).indices.data(), sizeof(float) * render_verts.size(), sizeof(unsigned int) * (*rec).indices.size(), rec->VAO, rec->VBO);
	}
	
	std::vector<float> cir_render_verts = world_space_to_render_space(cir.vertices);
	//std::vector<float> ar_verts = apply_aspect_ratio_to_vertices(cir_render_verts);
	//std::vector<float> ar_verts = cir_render_verts;
	render->create_VAO(cir_render_verts.data(), sizeof(float) * cir_render_verts.size(), cir.VAO, cir.VBO);

	render->set_shader("Vertex_Shader.txt", "Fragment_Shader.txt");

	//GAME LOOP
	while (!glfwWindowShouldClose(render->window)){
		dt = render->start_rendering(show_imgui, border_recs, cir, scores, x, y);

		//update player bumper pos
		if (w_key_state == true) {
			player_bumper->move_rectangle(0.0f, 6000.0f * dt);
			std::vector<float> render_verts = world_space_to_render_space(player_bumper->vertices);
			render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), (*player_bumper).VAO, (*player_bumper).VBO);
		}
		if (s_key_state == true) {
			player_bumper->move_rectangle(0.0f, -6000.0f * dt);
			std::vector<float> render_verts = world_space_to_render_space(player_bumper->vertices);
			render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), (*player_bumper).VAO, (*player_bumper).VBO);
		}

		
		//calculate ai movement
		calculate_ai_movement(&right_bumper, &cir);

		//update ball pos
		cir.move_circle(cir.vel_x * dt, cir.vel_y * dt);
		std::vector<float> render_verts = world_space_to_render_space(cir.vertices);
		render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), cir.VAO, cir.VBO);
		
		/*
		cir.center[1] = left_bumper.center[1];
		cir.vertices.clear();
		cir.calculate_vertices(cir.center);
		std::vector<float> render_verts = world_space_to_render_space(cir.vertices);
		render->change_VBO_Vertices(render_verts.data(), sizeof(float) * render_verts.size(), cir.VAO, cir.VBO);
		*/

		//check for collision with borders
		rectangle* collided_border = get_collision_with_borders({&top_border, &bottom_border}, &cir, rect_projections);
		if (collided_border != NULL){
			std::cout << "collision with border detected" << '\n';
			std::array<float, 2> ortho = get_orthogonal_vector(rect_projections[collided_border->center]);
			ortho = {abs(ortho[0]), abs(ortho[1])};
			std::array<float, 2> diff_vector = component_wise_multiplication({ cir.vel_x, cir.vel_y }, ortho);
			diff_vector = scalar_multiplication(diff_vector, 2);
			std::array<float, 2> new_vel_vector = difference_between_vectors(diff_vector, {cir.vel_x, cir.vel_y});
			cir.vel_x = new_vel_vector[0];
			cir.vel_y = new_vel_vector[1];
		}

		rectangle* collided_score_rect = get_collision_with_borders({ &right_border, &left_border }, &cir, rect_projections);
		if (collided_score_rect != NULL) {
			std::cout << "Collision with left or right border detected" << '\n';
			if (collided_score_rect->center[0] < 0) { //if collision with left border
				scores[1] = scores[1] + 1; //add score to the ai 
			}
			else {
				scores[0] = scores[0] + 1; //add score to the player
			}
			//reset the position and velocity of the ball
			cir.center = { 0.0f, 0.0f };
			cir.vel_x = 3000.0f;
			cir.vel_y = 0.0f;
		}

		//check collision with bumpers
		rectangle* collided_rect = get_collision_with_bumpers({ &right_bumper, &left_bumper }, &cir);
		if (collided_rect != NULL){
			std::cout << "collision with bumper detected" << '\n';
			float comp = (cir.center[1] - collided_rect->center[1]) / collided_rect->height;
			if (collided_rect->center[0] == -7500.0f) {
				cir.vel_x = 3000.0f;
				cir.vel_y = comp * 3000.0f;
			}
			else {
				cir.vel_x = -3000.0f;
				cir.vel_y = comp * 3000.0f;
			}
		}
	}
	render->wrapper->shutdown();
	glfwTerminate();
	std::cout << "goodbye" << '\n';
}
