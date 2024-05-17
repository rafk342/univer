#pragma once

#include "vendor/SFML/Graphics.hpp"
#include "base/RenderRequests.h"
#include "helpers/Helpers.h"
#include "base/sfGlobals.h"


class Line
{
	sf::RectangleShape line;

	sf::Vector2f start{};
	sf::Vector2f end{};

	float thickness = 1;
	float length = 0;
	float angle = 0;

	void RecalculateShape();

public:

	Line() = default;
	Line(sf::Vector2f start, sf::Vector2f end);
	Line(sf::Vector2f start, sf::Vector2f end, float thickness);
	Line(float thickness);

	void SetEndPoint(sf::Vector2f end);
	void SetStartPoint(sf::Vector2f start);
	void SetColor(sf::Color col);
	void SetThickness(float v);
	void DrawLine();
	void DrawLine(sf::Vector2f start, sf::Vector2f end);
};


