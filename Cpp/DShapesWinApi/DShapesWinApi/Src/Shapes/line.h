#pragma once

/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Shape.h"

class Line : public Shape
{
public:
	Line();

	void setPoint(POINT start, POINT end);

	// Draw
	void print(HDC hdc);
	void isNotSelectedPrint(HDC hdc);
	void isSelectedPrint(HDC hdc);
	bool select(POINT p);
	~Line();
};

