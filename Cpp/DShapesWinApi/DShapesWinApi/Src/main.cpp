/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

#include "Shapes/circle.h"
#include "Shapes/line.h"
#include "Shapes/rect.h"

#include "resource.h"

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <list>
#include <locale.h>
#include <vector>

#include "Utils/random.h"

using namespace std;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCSTR lpszClass = "Danee1: Vector graphics 2D demo";

int fontHeight = 0;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdParam, int nCmdShow) 
{
	Utils::Random<unsigned char> inst = Utils::Random<unsigned char>(134541);

	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
	return 0;
}

void init();

void wmCharStr(HWND hWnd, WPARAM wParam, Shape * s);
void wmImeStr(HWND hWnd, LPARAM lParam, Shape * s);
void transformCoordinte(POINT & p, double zoomRange, POINT startPoint);

bool isCreating = FALSE;
bool isSelected = FALSE;
bool isResizing = FALSE;
bool isMoving = FALSE;
bool bCapture;
int zoom;
XFORM XForm = { 1, 0, 0, 1, 0, 0 };

double zoomRange[] = { 0.25, 0.5 , 1, 1.25, 1.5, 2 ,3 };

POINT startPos;
list<Shape *> lt;
Shape * shape;

static string utf16ToUTF8(const wstring &s)
{
	const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, 
		0, NULL);

	vector<char> buf(size);
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], size, 0, NULL);

	return string(&buf[0]);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rt;

	static POINT start, old, end;
	static bool clicked = FALSE;
	static int position = 0;

	static list<Shape *>::iterator selectLt;

	int xInc, yInc;

	HPEN hPen, oldPen;

	HFONT hFont, OldFont;
	HFONT g_hfFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;

	switch (iMessage) {
	case WM_CREATE:
		startPos = { 0, };
		zoom = 2;
		return 0;
	case WM_COMMAND:
		switch (wParam) {
		case 40001: 
			shape = new Circle();
			isCreating = TRUE;
			isMoving = FALSE;
			isResizing = FALSE;
			break;
		case 40002:
			shape = new Rect();
			isCreating = TRUE;
			isMoving = FALSE;
			isResizing = FALSE;
			break;
		case 40003:
			shape = new Line();
			isCreating = TRUE;
			isMoving = FALSE;
			isResizing = FALSE;
			break;
		}
		return 0;
	case WM_LBUTTONDOWN:

		start.x = LOWORD(lParam);
		start.y = HIWORD(lParam);

		transformCoordinte(start, zoomRange[zoom], startPos);
		old.x = start.x;
		old.y = start.y;
		clicked = TRUE;

		if (isCreating) {
			isSelected = TRUE;
			return 0;
		}

		if (isSelected) {
			auto i_lt = lt.begin();
			for (int i = 0; i < position; i++) {
				if ((*i_lt)->select(start)) {
					position = i;
					isMoving = TRUE;
					isResizing = FALSE;
					selectLt = i_lt;
					return 0;
				}
				i_lt++;
			}

			if ((*selectLt)->selectPoint(start) >= 1
				&& (*selectLt)->selectPoint(start) <= 2) {
				isResizing = TRUE;
				isMoving = FALSE;
				return 0;
			}

			if ((*selectLt)->select(start)) {
				isResizing = FALSE;
				isMoving = TRUE;
			}
			else {
				for (int i = position; i < lt.size(); i++) {
					if ((*i_lt)->select(start)) {
						position = i;
						isMoving = TRUE;
						isResizing = FALSE;
						selectLt = i_lt;
						return 0;
					}
					i_lt++;
				}
				position = lt.size();
				isSelected = FALSE;
			}
		}
		else {
			position = 0;
			for (auto i_lt = lt.begin(); i_lt != lt.end(); i_lt++)
			{
				if ((*i_lt)->select(start)) {
					isSelected = TRUE;
					isMoving = TRUE;
					isResizing = FALSE;
					selectLt = i_lt;
					break;
				}
				position++;
			}

			if (position == lt.size())
			{
				isMoving = FALSE;
				isResizing = FALSE;
				isSelected = FALSE;
			}
		}
		return 0;
	case WM_RBUTTONDOWN:
		start.x = LOWORD(lParam);
		start.y = HIWORD(lParam);

		old.x = start.x;
		old.y = start.y;
		bCapture = TRUE;

		return 0;
	case WM_MOUSEMOVE: {
		end.x = LOWORD(lParam);
		end.y = HIWORD(lParam);
		hdc = GetDC(hWnd);

		if (bCapture) {
			startPos.x += (end.x - old.x);
			startPos.y += (end.y - old.y);
			InvalidateRgn(hWnd, NULL, TRUE);
			old.x = end.x;
			old.y = end.y;
			return 0;
		}

		if (clicked) {
			transformCoordinte(end, zoomRange[zoom], startPos);
			if (isCreating) {
				shape->setPoint(start, end);
			}
			if (isMoving) {
				(*selectLt)->move(old, end);
			}
			if (isResizing) {
				(*selectLt)->resize(old, end);
			}
			old.x = end.x;
			old.y = end.y;
			InvalidateRgn(hWnd, NULL, TRUE);
			return 0;
			ReleaseDC(hWnd, hdc);
		}
	}
	case WM_RBUTTONUP:
		bCapture = FALSE;
		return 0;
	case WM_LBUTTONUP:
		end.x = LOWORD(lParam);
		end.y = HIWORD(lParam);
		transformCoordinte(end, zoomRange[zoom], startPos);

		clicked = FALSE;
		if (isCreating) {
			shape->setPoint(start, end);
			lt.push_front(shape);
			selectLt = lt.begin();
			position = 0;
			isCreating = FALSE;
		}
		if (isMoving) {
			(*selectLt)->move(old, end);
		}
		if (isResizing) {
			(*selectLt)->resize(old, end);
		}

		isMoving = FALSE;
		isResizing = FALSE;

		InvalidateRgn(hWnd, NULL, TRUE);
		return 0;

	case WM_MOUSEWHEEL: {
		if ((short)HIWORD(wParam) == 120)
		{
			if (zoom >= (sizeof(zoomRange) / sizeof(double)) - 1)
				return 0;
			zoom++;
		}
		else {
			if (zoom == 0)
				return 0;
			zoom--;
		}

		InvalidateRgn(hWnd, NULL, TRUE);

		return 0;
	}
	case WM_PAINT: {
		wchar_t wstr[50];
		_itow_s(zoomRange[zoom] * 100, wstr, 10);
		wcscat_s(wstr, L"%");
		hdc = BeginPaint(hWnd, &ps);
		std::string utf8String = utf16ToUTF8(wstr);
		LPCSTR lpStr = utf8String.c_str();
		TextOut(hdc, 10, 10, lpStr, wcslen(wstr));
		GetClientRect(hWnd, &rt);
		int prevGraphicsMode = SetGraphicsMode(hdc, GM_ADVANCED);
		XForm.eM11 = zoomRange[zoom];
		XForm.eM22 = zoomRange[zoom];
		XForm.eDx = startPos.x;
		XForm.eDy = startPos.y;
		SetWorldTransform(hdc, &XForm);

		GetObject(g_hfFont, sizeof(LOGFONT), &lf);
		hFont = CreateFontIndirect(&lf);
		OldFont = (HFONT)SelectObject(hdc, hFont);
		int cnt = 0;
		if (!lt.empty()) {
			//
			for (auto i_lt = lt.rbegin(); i_lt != lt.rend(); i_lt++)
			{
				if (cnt == (lt.size() - position - 1))
				{
					(*i_lt)->isSelectedPrint(hdc);
				}
				else {
					(*i_lt)->isNotSelectedPrint(hdc);
				}
				cnt++;
			}
		}

		if (isCreating)
			shape->isSelectedPrint(hdc);

		SelectObject(hdc, OldFont);
		DeleteObject(hFont);
		SetGraphicsMode(hdc, prevGraphicsMode);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_CHAR: {
		auto i_lt = lt.begin();
		for (int i = 0; i < position; i++)
			i_lt++;
		if (typeid(*i_lt) == typeid(Line))
			return 0;
		if (lt.empty())
			return 0;
		wmCharStr(hWnd, wParam, (*i_lt));
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_IME_COMPOSITION: {
		auto i_lt = lt.begin();
		for (int i = 0; i < position; i++)
			i_lt++;
		if (typeid(*i_lt) == typeid(Line))
			return 0;
		if (lt.empty())
			return 0;
		wmImeStr(hWnd, lParam, (*i_lt));
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

//
void init()
{
	isCreating = FALSE;
	isSelected = FALSE;
	isResizing = FALSE;
	isMoving = FALSE;

	startPos = { 0, };
	zoom = 2; // 100% 

	for (list<Shape *>::iterator it = lt.begin(); it != lt.end(); it++)
		delete (*it);
	lt.clear();
}

// WM_CHAR
void wmCharStr(HWND hWnd, WPARAM wParam, Shape * s)
{
	HDC hdc;
	wstring str;

	SIZE sz;
	int p = s->getPos();

	hdc = GetDC(hWnd);
	switch (wParam) {
	case VK_RETURN:
		s->getStr().insert(p, L"\n");
		p++;
		s->setPos(p);
		break;
	default:
		str = wParam;
		if ((s->getStr()).length() == 0) {
			s->setStr(str);

			p++;
			s->setPos(p);

			break;
		}
		s->getStr().insert(p, str);
		p++;
		s->setPos(p);
		break;
	}
	ReleaseDC(hWnd, hdc);
}
// IME
void wmImeStr(HWND hWnd, LPARAM lParam, Shape * s) {
	HDC hdc;
	HIMC hIMC;
	POINT pt;

	static bool firstInput = TRUE;

	static wstring oldStr;
	int p = s->getPos();

	hdc = GetDC(hWnd);
	hIMC = ImmGetContext(hWnd);	// IME 
	if (hIMC == NULL) {
		return;
	}

	int cLen = 0;
	wchar_t comp[4] = { 0, };
	// IME 
	if (lParam & GCS_RESULTSTR) {
		cLen = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);
		if (cLen > 0) {
			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, comp, cLen);
			comp[cLen] = 0;	

			(s->getStr()).erase(p, oldStr.length());
			(s->getStr()).insert(p, comp);
			firstInput = TRUE;
			p++;
			s->setPos(p);
		}
	}
	else if (lParam & GCS_COMPSTR) {	// IME 
		cLen = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);
		if (cLen > 0) {
			ImmGetCompositionStringW(hIMC, GCS_COMPSTR, comp, cLen);
			if (firstInput) {
				oldStr = comp;
				firstInput = FALSE;
			}
			else {
				(s->getStr()).erase(p, oldStr.length());
			}
			(s->getStr()).insert(p, comp);
		}
	}
	// IME 
	ImmReleaseContext(hWnd, hIMC);
	ReleaseDC(hWnd, hdc);
}
// 
void transformCoordinte(POINT & p, double zoomRange, POINT startPoint)
{
	p.x = (p.x - startPoint.x) / zoomRange;
	p.y = (p.y - startPoint.y) / zoomRange;
}