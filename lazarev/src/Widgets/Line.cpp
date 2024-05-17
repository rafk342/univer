#include "Line.h"

void Line::RecalculateShape()
{
	length = helpers::lengthBetweenPoints(start, end);
	angle = helpers::angleBetweenPoints(start, end);
	line.setPosition(start);
	line.setRotation(angle);
	line.setSize({ length, thickness });
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Line::Line(sf::Vector2f start, sf::Vector2f end) 
	: start(start)
	, end(end)
{
	RecalculateShape();
	line.setFillColor(sfGlobals::globalLinesColor);
}

Line::Line(sf::Vector2f start, sf::Vector2f end, float thickness)
	: start(start)
	, end(end)
	, thickness(thickness)
{
	RecalculateShape();
	line.setFillColor(sfGlobals::globalLinesColor);
}


Line::Line(float thickness) : thickness(thickness)
{
	line.setFillColor(sfGlobals::globalLinesColor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Line::SetEndPoint(sf::Vector2f point)
{
	end = point;
	RecalculateShape();
}

void Line::SetStartPoint(sf::Vector2f point)
{
	start = point;
	RecalculateShape();
}

void Line::SetColor(sf::Color col)
{
	line.setFillColor(col);
}

void Line::DrawLine()
{
	RenderRequests::InvokeWidgetUpdate([this]
		{
			RenderRequests::getWindow()->draw(line);
		});
}

void Line::DrawLine(sf::Vector2f start_point, sf::Vector2f end_point)
{
	start = start_point;
	end = end_point;
	RecalculateShape();
	DrawLine();
}

void Line::SetThickness(float v)
{
	thickness = v;
	line.setSize({ length, thickness });
}


