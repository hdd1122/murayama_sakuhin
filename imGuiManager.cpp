#define IMGUI_ENABLE_DOCKING  // ← Docking機能を使いたいならこれが必要！
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <stdio.h>

#include "ImGuiManager.h"

void ImGuiManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    // ImGui IO取得
    ImGuiIO& io = ImGui::GetIO();

    //フォント読み込み 日本語対応＋Unicode対応
    ImFont* font = io.Fonts->AddFontFromFileTTF("asset/fonts/NotoSansJP-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    if (!font) {
        OutputDebugStringA("フォントの読み込みに失敗しました\n");
    }


    // UTF-8対応（デフォルトなのでたいていOKだけど念のため）
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.FontDefault = font;

    //io.Fonts->Build();
}

void ImGuiManager::Begin() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

}

void ImGuiManager::End() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
