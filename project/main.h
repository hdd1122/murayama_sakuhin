#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <functional>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")


#include <DirectXMath.h>
using namespace DirectX;

#include "DirectXTex.h"
#if _DEBUG
#pragma comment(lib,"DirectXTex_Debug.lib")
#else
#pragma comment(lib,"DirectXTex_Release.lib")
#endif

#include"vector3.h"


#pragma comment (lib, "winmm.lib")

#include <memory>


class ScreenSize
{
public:
    static int ScreenWidth;
    static int ScreenHeight;

    static void Init()
    {
        // ここで実行時のデスクトップ解像度を取得して、値を上書きする
        int monitorWidth = GetSystemMetrics(SM_CXSCREEN);
        int monitorHeight = GetSystemMetrics(SM_CYSCREEN);

        ScreenWidth = std::min(monitorWidth, 1920);
        ScreenHeight = std::min(monitorHeight, 1080);
    }

};




HWND GetWindow();

void Invoke(std::function<void()> Function, int Time);


class MouseInput
{
public:
    // publicなメンバー関数を追加
    static void Init();
    static void AddDelta(int dx, int dy);
    static void BeginUpdate();
    static int GetDeltaX() { return m_deltaX; }
    static int GetDeltaY() { return m_deltaY; }

    // UI操作モードの切り替え
    static void SetUIMode(bool isUIMode);
  
    // ゲッターを2種類用意
    static POINT GetVisualCursorPos() { return m_visualCursorPos; }  // 表示用
    static POINT GetLogicalCursorPos() { return m_logicalCursorPos; } // 計算用


private:
    // カメラ操作用
    static int m_deltaX;
    static int m_deltaY;

    // 2種類の座標を保持
    static POINT m_visualCursorPos;
    static POINT m_logicalCursorPos;
};


