#pragma once

/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include <iostream>
#include <string>
#include <windows.h>
#include "Shape.h"

class Circle : public Shape
{
private:
	POINT top    = {0,};
	POINT right  = {0,};
	POINT bottom = {0,};
	POINT left   = {0,};

	COLORREF color;

public:
	Circle();

	POINT getTop() { return this->top; }
	void  setTop(POINT p) { this->top = p; }
	POINT getRight() { return this->right; }
	void  setRight(POINT p) { this->right = p; }
	POINT getBottom() { return this->bottom; }
	void  setBottom(POINT p) { this->bottom = p; }
	POINT getLeft() { return this->left; }
	void  setLeft(POINT p) { this->left = p; }

	// Draw
	void isNotSelectedPrint(HDC hdc);
	void isSelectedPrint(HDC hdc);
	bool select(POINT p);
	~Circle();
};

