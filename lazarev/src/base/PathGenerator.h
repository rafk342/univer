#pragma once
#include <iostream>
#include <vector>

#include "vendor/SFML/Graphics.hpp"
#include "Widgets/Line.h"

#if 0
class Line;

class PathGenerator
{
public:
	enum direction
	{
		up,
		down,
		right,
		left
	};

	enum class Axis { X, Y };

	static Axis verify_direction(sf::Vector2i start, sf::Vector2i end, direction& requested_direction)
	{
		int dx = end.x - start.x;
		int dy = -(end.y - start.y);

		if (start.x == end.x) requested_direction = up;
		if (start.y == end.y) requested_direction = right;
		
		if (requested_direction == right)	dx > 0 ? requested_direction : requested_direction = left;
		if (requested_direction == left)	dx < 0 ? requested_direction : requested_direction = right;

		if (requested_direction == up)		dy > 0 ? requested_direction : requested_direction = down;
		if (requested_direction == down)	dy < 0 ? requested_direction : requested_direction = up;


		if (requested_direction == up || requested_direction == down) return Axis::Y;
		if (requested_direction == right || requested_direction == left) return Axis::X;
	}

	template<direction start_direction>
	static std::vector<Line> GenerateLinesBetweenPoints(sf::Vector2i start, sf::Vector2i end, float thickness = 2)
	{
		std::vector<Line> generated_lines;
		direction m_direction = start_direction;

		//			  y
		//			|
		//			|
		//			|              x
		// -------------------------
		//			|      .start
		//			|	   |      
		//			|	   |______.end

		Axis axis_to_move = verify_direction(start, end, m_direction);

		if (axis_to_move == Axis::X)
		{
			sf::Vector2f first_line_end = sf::Vector2f(end.x, start.y);
			generated_lines.push_back(Line(sf::Vector2f(start), first_line_end, thickness));

			if (first_line_end.y != end.y)
				generated_lines.push_back(Line(first_line_end, sf::Vector2f(end.x , end.y), thickness));

		}

		return generated_lines;
	}
};

#endif