/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Shape.h"


Shape::Shape(){}


Shape::~Shape(){}


int Shape::drawTextCenter(HDC dc, LPCSTR text, int cchText, LPCRECT lprc, UINT format)
{
	if (dc == NULL || lprc == NULL) return 0;

	RECT rect = *lprc;
	RECT rectResult = *lprc;

	DrawText(dc, text, cchText, &rect, format | DT_CALCRECT);

	int marginH = max(0, ((lprc->bottom - lprc->top) - (rect.bottom - rect.top)) / 2);
	int marginW = max(0, ((lprc->right - lprc->left) - (rect.right - rect.left)) / 2);
	rectResult.top += marginH;
	rectResult.bottom -= marginH;
	rectResult.left += marginW;
	rectResult.right -= marginW;

	DrawText(dc, text, cchText, &rectResult, format);
	return rect.bottom;
}

void Shape::isNotSelectedPrint(HDC hdc)
{
}

void Shape::isSelectedPrint(HDC hdc)
{
}

bool Shape::select(POINT p)
{
	return false;
}


int Shape::selectPoint(POINT p)
{

	if (abs(p.x - p1.x) <= 20 && abs(p.y - p1.y) <= 20)
	{
		return 1;
	}
	else if (abs(p.x - p2.x) <= 20 && abs(p.y - p2.y) <= 20)
	{
		return 2;
	}
	return 0;
}


void Shape::resize(POINT start, POINT end)
{
	if (abs(start.x - p1.x) <= 20 && abs(start.y - p1.y) <= 20)
	{  
		p1.x = end.x;
		p1.y = end.y;
	}
	else if (abs(start.x - p2.x) <= 20 && abs(start.y - p2.y) <= 20) {
		p2.x = end.x;
		p2.y = end.y;
	}
}


void Shape::move(POINT start, POINT end)
{
	int distanceX = end.x - start.x;
	int distanceY = end.y - start.y;
	p1.x += distanceX;
	p1.y += distanceY;
	p2.x += distanceX;
	p2.y += distanceY;
}


void Shape::setPoint(POINT start, POINT end)
{

	if (start.x <= end.x && start.y <= end.y) {
		p1 = start;
		p2 = end;
	}

	else if (start.x >= end.x && start.y >= end.y) {
		p1 = end;
		p2 = start;
	}

	else if (start.x >= end.x && start.y <= end.y) {
		p1.x = end.x;
		p1.y = start.y;
		p2.x = start.x;
		p2.y = end.y;
	}

	else {
		p1.x = start.x;
		p1.y = end.y;
		p2.x = end.x;
		p2.y = start.y;
	}
}