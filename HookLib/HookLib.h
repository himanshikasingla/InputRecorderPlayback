#pragma once
#include <windows.h>
#include <string>

extern HHOOK hMouseHook;
extern HHOOK hKeyboardHook;
extern HINSTANCE hInst;
extern std::wstring g_filePath;

extern "C" __declspec(dllexport) void InstallHooks(HWND hWnd, const wchar_t* filePath);
extern "C" __declspec(dllexport) void UninstallHooks();