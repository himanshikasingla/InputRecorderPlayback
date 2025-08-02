#include "pch.h"
#include "HookLib.h"
#include <fstream>
#include <chrono>

HHOOK hMouseHook = nullptr;
HHOOK hKeyboardHook = nullptr;
HINSTANCE hInst = nullptr;
std::wstring g_filePath;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0) {
        auto now = std::chrono::steady_clock::now();
        long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;

        std::wofstream out(g_filePath, std::ios::app);
        if (out.is_open()) {
            out << L"MOUSE " << wParam << L" " << pMouse->pt.x << L" " << pMouse->pt.y << L" " << timestamp << std::endl;
            out.close();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

        auto now = std::chrono::steady_clock::now();
        long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        std::wofstream out(g_filePath, std::ios::app);
        if (out.is_open()) {
            out << L"KEY " << pKey->vkCode << L" " << (wParam == WM_KEYUP ? L"UP" : L"DOWN") << L" " << timestamp << std::endl;
            out.close();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void InstallHooks(HWND hWnd, const wchar_t* filePath)
{
    g_filePath = filePath;
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInst, 0);
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);
}

extern "C" __declspec(dllexport) void UninstallHooks()
{
    if (hMouseHook) UnhookWindowsHookEx(hMouseHook);
    if (hKeyboardHook) UnhookWindowsHookEx(hKeyboardHook);
}
