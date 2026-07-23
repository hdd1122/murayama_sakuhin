#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "CheckPoint.h"
#include "player.h"
#include "camera.h"
#include "input.h"
#include "time.h"
#include <algorithm>
#include "scene.h"
#include "result.h"

#include "imgui.h"

//チェックポイント、ゴールにもなる

void CheckPoint::Init(int CPNum)//1スタートだよ
{
    m_modelRenderer = new ModelRenderer();
    m_modelRenderer->Load("asset\\CheckPoint.obj");

    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader\\checkPointOitAccum.cso");

    m_color = { 0.5f,1.0f,0.5f,0.1f };

    m_isActive = true;

	m_scale = { 10.0f,200.0f,10.0f };
	//チェックポイントの位置設定
	m_position.x = cpos[CPNum - 1].x;
	m_position.y = cpos[CPNum - 1].y;
	m_position.z = cpos[CPNum - 1].z;

	m_meNum = CPNum;

    if (CPNum == CheckPointMaxNum)
    {
        m_isGoal = true;
        m_color = { 1.0f, 1.0f, 0.5f, 0.1f };
    }
    else
		m_isGoal = false;
}

void CheckPoint::Uninit()
{
    delete m_modelRenderer;
    m_VertexShader->Release();
    m_PixelShader->Release();
    m_VertexLayout->Release();
}

//後は判定と位置調整でOK

void CheckPoint::Update()
{
    if (m_isActive)
    {
        auto* player = Manager::GetScene()->GetGameObject<Player>();
		Vector3 pPos = player->GetPos();

		//プレイヤーとの距離計算
        float vecX = (pPos.x - m_position.x) * (pPos.x - m_position.x);
        float vecZ = (pPos.z - m_position.z) * (pPos.z - m_position.z);

        //プレイヤーとの判定
		if (vecX + vecZ < m_scale.x * m_scale.x || test)//testはimgui用
        {
            if (m_meNum != CheckPointMaxNum)//ゴールでは無い場合
            {
                //プレイヤーのresetPosを変更
                player->SetResetPos(m_position);
                //次のチェックポイントを生成（この時に自分の番号＋１をinitかなんかの引数で与える）
                //自分の番号はシーンかなんかが持つべきかも
                Manager::GetScene()->AddGameObject<CheckPoint>(l_HANTOUMEI_BLOOM)->Init(m_meNum + 1);

                //自分の削除
                //削除まで時間が掛かるため、drawやupdateは行わないようにする。
                SetDestroy();
            }
            else//ゴールの場合
            {
                Manager::SetScene<Result>();
            }

            m_isActive = false;
        }
        
    }

}


void CheckPoint::Draw()
{
    if (m_isActive)
    {

        // シェーダーとインプットセット
        Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
        Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
        Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

        XMMATRIX world, scale, rot, trans;
        scale = XMMatrixScaling(m_scale.x, m_scale.y + m_scale.y, m_scale.z);
        rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
        trans = XMMatrixTranslation(m_position.x, m_position.y + m_scale.y, m_position.z);
        world = scale * rot * trans;

        Renderer::SetWorldMatrix(world);

		//ブルームカラーの設定

        BloomColor bc;
        bc.bloomColor = XMFLOAT4(m_color.x, m_color.y, m_color.z, 0.3f);
        Renderer::UpdateBloomColor(bc);


        // 外壁の描画
        // ラスタライザーステートをカリング無しに設定
        Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsNull());

        // 外壁用のマテリアル（薄めの色）を設定
        MATERIAL materialOuter = {};
        materialOuter.Diffuse = m_color;
        Renderer::SetMaterial(materialOuter);

        // 描画
        m_modelRenderer->DrawOitAccumulation();

        //設定戻す
        Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());


    }
}



void CheckPoint::DrawImgui()
{
    // IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
    ImGui::PushID(m_No);

    // ヘッダーのラベルだけは番号を表示したいので sprintf が必要
    char headerName[32];
    sprintf(headerName, u8"checkPoint %d", m_meNum);

    if (ImGui::CollapsingHeader(headerName))
    {
        // ここは単純に "Position" だけ
        // PushID のおかげで、内部的には "Position/1", "Position/2" のように区別される
        ImGui::DragFloat3("Position", &m_position.x, 0.1f);
        ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
        test = ImGui::Button("check");

    }

    // 最後に必ず PopID する
    ImGui::PopID();
}


