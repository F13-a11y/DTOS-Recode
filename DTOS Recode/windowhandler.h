#pragma once
#include <windows.h>
#define STANDARD GetModuleHandle(NULL)
void WindowHandler(HINSTANCE hInstance, const wchar_t* title);