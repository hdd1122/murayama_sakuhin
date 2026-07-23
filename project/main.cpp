#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "main.h"
#include "manager.h"
#include <thread>
#include "ImGuiManager.h"
#include "renderer.h" 
#include <windowsx.h>
#include <algorithm>  
#include "input.h"

const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11ゲーム";

// --- マウス関連 ---

// staticメンバー変数の実体定義
int MouseInput::m_deltaX = 0;
int MouseInput::m_deltaY = 0;
POINT MouseInput::m_visualCursorPos = { 0, 0 };
POINT MouseInput::m_logicalCursorPos = { 0, 0 }; 

//screenSizeのinitで初期値はデバイスごとに変化
int ScreenSize::ScreenWidth = 1280;//暫定初期値
int ScreenSize::ScreenHeight = 720;

void MouseInput::Init()
{
	m_deltaX = 0;
	m_deltaY = 0;
	// 初期位置を画面中央に設定
	m_visualCursorPos = { ScreenSize::ScreenWidth / 2, ScreenSize::ScreenHeight / 2 };
	m_logicalCursorPos = { ScreenSize::ScreenWidth / 2, ScreenSize::ScreenHeight / 2 };

	//
	SetUIMode(false);

}

void MouseInput::BeginUpdate()
{
	// 計算用の座標を更新 (クランプしない)
	m_logicalCursorPos.x += m_deltaX;
	m_logicalCursorPos.y += m_deltaY;

	// 表示用の座標を更新 (クランプする)
	m_visualCursorPos.x += m_deltaX;
	m_visualCursorPos.y += m_deltaY;
	m_visualCursorPos.x = std::max(0L, std::min(m_visualCursorPos.x, (LONG)ScreenSize::ScreenWidth - 1));
	m_visualCursorPos.y = std::max(0L, std::min(m_visualCursorPos.y, (LONG)ScreenSize::ScreenHeight - 1));

	// 次のフレームのために移動量をリセット
	m_deltaX = 0;
	m_deltaY = 0;


}

void MouseInput::AddDelta(int dx, int dy)
{
	m_deltaX += dx;
	m_deltaY += dy;
}

void MouseInput::SetUIMode(bool isUIMode)
{
	if (isUIMode)
	{
		// UIモード：本物のカーソルを中央に固定して隠す
		ShowCursor(TRUE);
		RECT rect;
		GetClientRect(GetWindow(), &rect);
		ClientToScreen(GetWindow(), (POINT*)&rect.left);
		ClientToScreen(GetWindow(), (POINT*)&rect.right);

		// 中央の座標を計算
		int centerX = rect.left + (rect.right - rect.left) / 2;
		int centerY = rect.top + (rect.bottom - rect.top) / 2;
		SetCursorPos(centerX, centerY);

		ClipCursor(&rect); // ウィンドウ内にロック
	}
	else
	{
		// ゲームプレイモード：カーソルを非表示にしてロック
		ShowCursor(FALSE);
#ifdef _DEBUG
		ShowCursor(TRUE);
#endif // _DEBUG


		RECT rect;
		GetClientRect(GetWindow(), &rect);
		ClientToScreen(GetWindow(), (POINT*)&rect.left);
		ClientToScreen(GetWindow(), (POINT*)&rect.right);

		ClipCursor(&rect);
	}
}

//
//------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HWND g_Window;

HWND GetWindow()
{
	return g_Window;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	SetProcessDPIAware();
	
	ScreenSize::Init();

	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)ScreenSize::ScreenWidth, (LONG)ScreenSize::ScreenHeight };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);


	Manager::Init();
#ifdef NDEBUG

	//Renderer::ToggleFullscreen(g_Window);
#endif
	Renderer::ToggleFullscreen(g_Window);

	//imgui初期化
	ImGuiManager::Init(g_Window, Renderer::GetDevice(), Renderer::GetDeviceContext());
	//マウスクラス初期化
	MouseInput::Init();

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01; // Generic Desktop Controls
	rid.usUsage = 0x02;     // Mouse
	rid.dwFlags = 0;        // デフォルトの挙動
	rid.hwndTarget = g_Window;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));



	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);




	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;



	MSG msg;
	while(1)
	{
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if((dwCurrentTime - dwExecLastTime) >= (1000 / 240))
			{
				dwExecLastTime = dwCurrentTime;

				Manager::Update();
				Manager::Draw();

				MouseInput::BeginUpdate();
		

			}
		}
	}

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	ImGuiManager::Shutdown();

	Manager::Uninit();

	CoUninitialize();

	return (int)msg.wParam;
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ImGuiにメッセージを渡す。ImGuiがメッセージを処理したらtrueが返る。
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true; // ImGuiが処理したので、このメッセージはここで終わり
	}

	switch(uMsg)
	{
	case WM_MOUSEWHEEL:
	{
		// GET_WHEEL_DELTA_WPARAM で回転量を取得
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		Input::SetWheel(delta);
		return 0; // 処理したので 0 を返す
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			
		}
		return 0;

	
	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & (1 << 29))) // Altキーが押されているか
		{
			Renderer::ToggleFullscreen(g_Window);
		}
		break;

		// ローマウスインプットのメッセージを処理
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));


		std::unique_ptr<BYTE[]> lpb = std::make_unique<BYTE[]>(dwSize);
		if (lpb == NULL) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = (RAWINPUT*)lpb.get();

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			// マウスの移動量をMOUSEMOVE_Oクラスに渡す
			MouseInput::AddDelta(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
		}

		break;
	}

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

