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

	void move_circle(float mov_x, float mov_y) {
		//update center
		center[0] = center[0] + mov_x;
		center[1] = center[1] + mov_y;

		//calculate vertices
		vertices.clear();
		calculate_vertices(center);
	}
};