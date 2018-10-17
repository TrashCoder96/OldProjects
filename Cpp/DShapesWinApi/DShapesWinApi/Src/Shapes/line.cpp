/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Line.h"

Line::Line()
{
}

void Line::isNotSelectedPrint(HDC hdc)
{
	MoveToEx(hdc, this->getPoint1().x , this->getPoint1().y, NULL);
	LineTo(hdc, this->getPoint2().x, this->getPoint2().y);
}

void Line::isSelectedPrint(HDC hdc)
{
	isNotSelectedPrint(hdc);
	Rectangle(hdc, getPoint1().x - 3, getPoint1().y - 3,
		getPoint1().x + 3, getPoint1().y + 3);
	Rectangle(hdc, getPoint2().x - 3, getPoint2().y - 3,
		getPoint2().x + 3, getPoint2().y + 3);
}

bool Line::select(POINT p)
{
	double result;
	POINT lowY, highY;
	POINT lowX, highX;

	if (getPoint1().y > getPoint2().y) {
		lowY = getPoint2();
		highY = getPoint1();
	} else {
		lowY = getPoint1();
		highY = getPoint2();
	}

	if (getPoint1().x > getPoint2().x) {
		lowX = getPoint2();
		highX = getPoint1();
	} else {
		lowX = getPoint1();
		highX = getPoint2();
	}

	if (getPoint1().x == getPoint2().x) {
		if (lowY.y <= p.y && highY.y >= p.y
			&& (lowY.x - 1) <= p.x && (lowY.x + 1) >= p.x)
			return TRUE;
		return FALSE;
	}
	else if (getPoint1().y == getPoint2().y) {
		if (lowX.x <= p.x && highX.x >= p.x
			&& (lowX.y - 1) <= p.y && (lowX.y + 1) >= p.y)
			return TRUE;
		return FALSE;
	}
	else {
		result = (highX.x - lowX.x) * p.y - (highX.y - lowX.y) * (p.x - lowX.x);
		if (p.x >= lowX.x && p.x <= highX.x
			&& p.y >= (lowY.y - 1) && p.y <= (highY.y + 1)
			&& result >= ((highX.x - lowX.x) * lowX.y - 1500) && result <= ((highX.x - lowX.x) * lowX.y + 1500)) {
			std::cout << "line select!" << std::endl;
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

//
void Line::setPoint(POINT start, POINT end)
{
	p1 = start;
	p2 = end;
}
Line::~Line()
{
}
