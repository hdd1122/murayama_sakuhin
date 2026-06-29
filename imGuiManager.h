#pragma once

#include <windows.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

class ImGuiManager {
public:
    static void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
    static void Begin();
    static void End();
    static void Shutdown();
};
