/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Circle.h"
#include <math.h>


Circle::Circle(): color(randomColor())
{
}

void Circle::isNotSelectedPrint(HDC hdc)
{
	HBRUSH hBrush, oldBrush;
	RECT rt;
	rt.left = p1.x;
	rt.top = p1.y;
	rt.right = p2.x;
 	rt.bottom = p2.y;

	hBrush = CreateSolidBrush(color);
	oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
	SelectObject(hdc, oldBrush);
	DeleteObject(hBrush);
	SetBkMode(hdc, TRANSPARENT);
}

void Circle::isSelectedPrint(HDC hdc)
{
	isNotSelectedPrint(hdc);
	Rectangle(hdc, p1.x - 3, p1.y - 3,
		p1.x + 3, p1.y + 3);
	Rectangle(hdc, p2.x - 3, p2.y - 3,
		p2.x + 3, p2.y + 3);
	Rectangle(hdc, p1.x - 3, p2.y - 3,
		p1.x + 3, p2.y + 3);
	Rectangle(hdc, p2.x - 3, p1.y - 3,
		p2.x + 3, p1.y + 3);
}

bool Circle::select(POINT p)
{
	if (((10000 * pow(2 * p.x - getPoint1().x - getPoint2().x, 2)) / pow((getPoint2().x - getPoint1().x), 2)
		+ (10000 * pow((2 * p.y - getPoint1().y - getPoint2().y), 2)) / pow((getPoint2().x - getPoint1().y), 2))
		<= 10000) {
		//std::cout << "circle true" << std::endl;
		return true;
	}
	return false;
}

Circle::~Circle()
{
}
