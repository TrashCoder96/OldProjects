#pragma once

/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <list>
#include <string>

#include "Utils/random.h"

using namespace std;


inline COLORREF randomColor() { 
	using  Utils::Random;
	static Random<unsigned char> instance(1294011);
	return RGB(instance.random(), instance.random(), instance.random());
}


class Shape
{
protected:

	POINT p1 = {0, 0};
	POINT p2 = {0, 0};
	wstring str = L"";
	int pos = 0;

	POINT originP1 = { 0, };
	POINT originP2 = { 0, };

	POINT pan = { 0, };

public:
	Shape();

	POINT getPoint1() { return this->p1; }
	void setPoint1(POINT p) { this->p1 = p; }
	POINT getPoint2() { return this->p2; }
	void setPoint2(POINT p) { this->p2 = p; }
	POINT getOriginPoint1() { return this->originP1; }
	void setOriginPoint1(POINT p) { this->originP1 = p; }
	POINT getOriginPoint2() { return this->originP2; }
	void setOriginPoint2(POINT p) { this->originP2 = p; }

	wstring& getStr() { return this->str; }
	void setStr(wstring str) { this->str = str; }
	int getPos() { return this->pos; }
	void setPos(int pos) { this->pos = pos; }

	// set & sort Point
	virtual void setPoint(POINT start, POINT end);

	// Draw
	virtual void isNotSelectedPrint(HDC hdc);
	virtual void isSelectedPrint(HDC hdc);
	virtual bool select(POINT p);
	int drawTextCenter(HDC dc, LPCSTR text, int cchText, LPCRECT lprc, UINT format);

	// changeSize
	int selectPoint(POINT p);
	void resize(POINT start, POINT end);

	void move(POINT start, POINT end);

	~Shape();
};
