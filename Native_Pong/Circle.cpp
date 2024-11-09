#define _USE_MATH_DEFINES
#pragma once
#include <array>
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>

struct Circle {
	std::array<float, 2> center;
	float radious;
	unsigned int VAO;
	unsigned int VBO;
	
	float vel_x = 3000.0f;
	float vel_y = 0.0f;
	//float vel_y = 3000.0f;
	std::vector<float> vertices;

	void create_circle(std::array<float, 2> pcenter, float prad) {
		center = pcenter;
		radious = prad;

		calculate_vertices(pcenter);
		//TODO debug stuff
		std::ofstream file;
		file.open("vertices.txt");
		for (int i = 1; i < vertices.size(); i = i + 3) {
			file << "x: " << vertices[i-1] << " y: " << vertices[i]<< '\n';
		}
	}

	void calculate_vertices(std::array<float, 2> pcenter){
		//add the pivot point first
		vertices.push_back(pcenter[0]);
		vertices.push_back(pcenter[1]);
		vertices.push_back(0.0f);
		
		int count {1};
		for (float angle = 0; angle < 2 * M_PI; angle = angle + 0.05) {
			count++;
			float point_x = center[0] + radious * cos(angle);
			float point_y = center[1] + radious * sin(angle);

			vertices.push_back(point_x);
			vertices.push_back(point_y);
			vertices.push_back(0.0f);
		}
	}

	void add_vertex_to_list(std::vector<float>& plist, float vertices[2]) {
		plist.push_back(vertices[0]);
		plist.push_back(vertices[1]);
		plist.push_back(0.0f);
	}

	void calculate_vertices_2(std::array<float, 2> pcenter) {
		float step = 0.05f;
		float last_param = 0.00f;
		float param = last_param + step;

		float centerx = center[0];
		float centery = center[1];

		//add_vertex_to_list(vertices, &center[0]);

		float point_1[2]{};
		float point_2[2]{};

		while (param <= 2) {
			float radians1 = last_param * M_PI;
			float radians2 = param * M_PI;

			point_1[0] = centerx + (radious * (std::cos(radians1)));
			point_1[1] = centery + (radious * (std::sin(radians1)));

			point_2[0] = centerx + (radious * (std::cos(radians2)));
			point_2[1] = centery + (radious * (std::sin(radians2)));

			add_vertex_to_list(vertices, point_1);
			add_vertex_to_list(vertices, point_2);

			last_param = param;
			param += step;
		}
	}

	void calculate_vertices_3(std::array<float, 2> pcenter) {
		//create two triangles
		//pivot point will be the center of the circle
		vertices.push_back(pcenter[0]);
		vertices.push_back(pcenter[1]);
		vertices.push_back(0.0f);

		//next point will be the radian angles on pi/2
		float point_x_2 = center[0] + radious * cos(M_PI / 2);
		float point_y_2 = center[1] + radious * sin(M_PI / 2);
		//next angle will be in 3*pi/4
		float point_x_3 = center[0] + radious * cos(3*M_PI / 4);
		float point_y_3 = center[1] + radious * sin(3*M_PI / 4);
		//next angle will be in pi
		float point_x_4 = center[0] + radious * cos(M_PI);
		float point_y_4 = center[1] + radious * sin(M_PI);
		
		//pass to vertex list
		vertices.push_back(point_x_2);
		vertices.push_back(point_y_2);
		vertices.push_back(0.0f);

		vertices.push_back(point_x_3);
		vertices.push_back(point_y_3);
		vertices.push_back(0.0f);

		//vertices.push_back(point_x_3);
		//vertices.push_back(point_y_3);
		//vertices.push_back(0.0f);

		vertices.push_back(point_x_4);
		vertices.push_back(point_y_4);
		vertices.push_back(0.0f);
	}

	void move_circle(float mov_x, float mov_y) {
		//update center
		center[0] = center[0] + mov_x;
		center[1] = center[1] + mov_y;

		//calculate vertices
		vertices.clear();
		calculate_vertices(center);
	}
};