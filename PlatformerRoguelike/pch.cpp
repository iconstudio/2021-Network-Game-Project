#include "pch.h"
#include "resource.h"


void Render::refresh(HWND hwnd) {
	InvalidateRect(hwnd, NULL, FALSE);
}

void Render::transform_set(HDC world, XFORM& info) {
	SetWorldTransform(world, &info);
}

void Render::transform_set_identity(HDC world) {
	SetWorldTransform(world, &transform_identity);
}

void Render::draw_end(HDC handle, HGDIOBJ object_old, HGDIOBJ object_default) {
	SelectObject(handle, object_old);
	DeleteObject(object_default);
}

BOOL Render::draw_rectangle(HDC hDC, int x1, int y1, int x2, int y2) {
	return Rectangle(hDC, x1, y1, x2, y2);
}

WindowsClient::WindowsClient(LONG cw, LONG ch)
	: width(cw), height(ch), procedure(NULL) {}

WindowsClient::~WindowsClient() {
	UnregisterClassW(class_id, instance);
}

BOOL WindowsClient::initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show) {
	properties.cbSize = sizeof(WNDCLASSEX);
	properties.style = CS_HREDRAW | CS_VREDRAW;
	properties.lpfnWndProc = procedure;
	properties.cbClsExtra = 0;
	properties.cbWndExtra = 0;
	properties.hInstance = handle;
	properties.hIcon = LoadIcon(handle, MAKEINTRESOURCE(IDI_PLATFORMERROGUELIKE));
	properties.hCursor = LoadCursor(nullptr, IDC_ARROW);
	properties.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	properties.lpszMenuName = MAKEINTRESOURCE(IDC_PLATFORMERROGUELIKE);
	properties.lpszClassName = id;
	properties.hIconSm = LoadIcon(properties.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&properties);


	HWND hWnd = CreateWindow(id, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
							  , CW_USEDEFAULT, 0, width, height, nullptr, nullptr, instance, nullptr);
	instance = handle;
	title_caption = title;
	class_id = id;

	if (!hWnd) {
		return FALSE;
	}

	hwindow = hWnd;
	ShowWindow(hWnd, cmd_show);
	UpdateWindow(hWnd);

	return TRUE;
}
