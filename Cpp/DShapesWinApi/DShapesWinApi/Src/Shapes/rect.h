#pragma once

/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Shape.h"
#include <string>

class Rect : public Shape
{
private:
	POINT right = {0,};
	POINT left = {0,};

	COLORREF color;
public:
	Rect();

	POINT getRight() { return this->right; }
	void  setRight(POINT p) { this->right = p; }
	POINT getLeft() { return this->left; }
	void  setLeft(POINT p) { this->left = p; }

	// Draw
	void isNotSelectedPrint(HDC hdc);
	void isSelectedPrint(HDC hdc);
	bool select(POINT p);
 	~Rect();
};

