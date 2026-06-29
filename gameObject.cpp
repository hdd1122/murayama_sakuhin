#include "main.h"
#include "gameObject.h"
#include "imgui.h"
#include <typeinfo>
#include <string>

bool GameObject::m_allImgui = false;

//imgui共通
void GameObject::DrawImgui()
{
    if (m_allImgui)
    {
        // IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
        ImGui::PushID(m_No);

        // ヘッダーのラベルだけは番号
        // を表示したいので sprintf が必要
        //クラス名で
        char headerName[32];
        std::string className = typeid(*this).name();
        sprintf(headerName, "%s %d", className.c_str(), m_No);


        if (ImGui::CollapsingHeader(headerName))
        {

            ImGui::DragFloat3("Position", &m_position.x, 0.1f);
            ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
            ImGui::DragFloat3("Rot", &m_rotation.x, 0.1f);

        }

        // 最後に必ず PopID する
        ImGui::PopID();
    }
}
