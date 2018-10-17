/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Rect.h"



Rect::Rect() : color(randomColor())
{
}

void Rect::isNotSelectedPrint(HDC hdc)
{
	HBRUSH hBrush, oldBrush;
	RECT rt;
	rt.left = p1.x;
	rt.top = p1.y;
	rt.right = p2.x;
	rt.bottom = p2.y;

	hBrush = CreateSolidBrush(color);
	oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	Rectangle(hdc, p1.x, p1.y, p2.x, p2.y);
	SelectObject(hdc, oldBrush);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
}

void Rect::isSelectedPrint(HDC hdc)
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

bool Rect::select(POINT p)
{
	POINT tmp1, tmp2;
	tmp1 = getPoint1();
	tmp2 = getPoint2();
	if (tmp1.x > tmp2.x && tmp1.y > tmp2.y) {
		setPoint1(tmp2);
		setPoint2(tmp1);
	}
	else if (tmp1.x <= tmp2.x && tmp1.y > tmp2.y) {
		tmp1.y = getPoint2().y;
		tmp2.y = getPoint1().y;
		setPoint1(tmp1);
		setPoint2(tmp2);
	}
	else if (tmp1.x > tmp2.x && tmp1.y <= tmp2.y) {
		tmp1.x = getPoint2().x;
		tmp2.x = getPoint1().x;
		setPoint1(tmp1);
		setPoint2(tmp2);
	}

	if ((p.x >= getPoint1().x) && (p.x <= getPoint2().x)
		&& (p.y >= getPoint1().y) && (p.y <= getPoint2().y)) {
		return TRUE;
	}
	return FALSE;
}

Rect::~Rect()
{
}
