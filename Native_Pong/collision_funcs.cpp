#pragma once
#include "circle.cpp"
#include <array>
#include <math.h>
bool is_point_inside_circle(std::array<float, 2> point, Circle* cir) {
	//get distance between center of ball and point given
	float dist_x = point[0] - (*cir).center[0];
	float dist_y = point[1] - (*cir).center[0];

	float distance = sqrt(pow(dist_x, 2) + pow(dist_y, 2));
	if ((*cir).radious <= distance) {
		return true;
	}
	return false;
}

bool is_point_in_line(std::array<float, 2> point, std::array<float,2 > line1, std::array<float,2> line2) {
	//get length of the line
	float line_length = sqrt(pow(line2[0] - line1[0], 2) + pow(line2[1] - line1[1], 2));
	//get distance of the point to both ends of the line
	float dist1 = sqrt(pow(point[0] - line1[0], 2) + pow(point[1] - line1[1], 2));
	float dist2 = sqrt(pow(point[0] - line2[0], 2) + pow(point[1] - line2[1], 2));
	//tolerance buffer to detect collisions
	float buffer = 0.1f;
	if (dist1 + dist2 <= line_length + buffer && dist1 + dist2 >= line_length - buffer) {
		return true;
	}
	return false;
}

bool circle_collision_with_line(std::array<float, 2 > line1, std::array<float, 2> line2, Circle* cir) {
	//check if either line endpoint is inside the circle
	bool inside_1 = is_point_inside_circle(line1, cir);
	bool inside_2 = is_point_inside_circle(line2, cir);
	if (inside_1 || inside_2) {
		return true;
	}
	//get the closest point in the line to the circle
	float dist_x = line1[0] - line2[0];
	float dist_y = line1[1] - line2[1];
	float length = sqrt(pow(dist_x, 2) + pow(dist_y, 2));

	float dot = ((((*cir).center[0] - line1[0]) * (line2[0] - line1[0])) + (((*cir).center[1] - line1[1]) * (line2[1] - line1[1]))) / sqrt(length);
	float closest_x = line1[0] + (dot * (line2[0] - line1[0]));
	float closest_y = line1[1] + (dot * (line2[1] - line1[1]));
	bool onlinesegment = is_point_in_line({closest_x, closest_y}, line1, line2);
	if (!onlinesegment) { return false; }
	//get the distance between the closest point and the center of the circle
	dist_x = closest_x - (*cir).center[0];
	dist_y = closest_y - (*cir).center[1];
	float distance = sqrt(pow(dist_x, 2) + pow(dist_y, 2));
	if (distance <= (*cir).radious) {
		return true;
	}
	return false;
}