#include <array>
#include <vector>
#pragma once
struct rectangle {
	std::array<float,2> center;
	float width;
	float height;
	
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	unsigned int VAO;
	unsigned int VBO;

	void create_rectangle(std::array<float, 2> pcenter, float pwidth, float pheight) {
		center = pcenter;
		width = pwidth;
		height = pheight;

		calculate_vertices();
	}

	void calculate_vertices() {
		float center_x = center[0]; 
		float center_y = center[1];
		std::array <float, 2> bottom_left = { center_x - (width / 2), center_y - (height / 2)};
		std::array<float, 2> bottom_right = {center_x + (width / 2), center_y - (height / 2)};
		std::array<float, 2> top_right = {center_x + (width / 2), center_y + (height / 2)};
		std::array<float, 2> top_left = {center_x - (width / 2), center_y + (height / 2)};

		vertices = {
					bottom_left[0] ,    bottom_left[1] , 0.0f,
					bottom_right[0],    bottom_right[1], 0.0f,
					top_right[0]   ,    top_right[1]   , 0.0f,
			        top_left[0]    ,    top_left[1]    , 0.0f
				   };
		indices = { 0, 1, 2, //right bottom triangle
					0, 2, 3  //top bottom triangle
				  };
	}

	void move_rectangle(float movement_x, float movement_y) {
		//move the center
		center[0] = center[0] + movement_x;
		center[1] = center[1] + movement_y;

		//recalculate vertices
		calculate_vertices();
	}

};