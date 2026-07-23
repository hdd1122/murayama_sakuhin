#include "main.h"
#include "manager.h"
#include "Cloud.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "imgui.h"
//オブジェクトとして配置する雲
//処理的に厳しい為現在描画しておらず




void Cloud::Init()
{

    m_noiseOffset.x = (rand() % 1000) / 10.0f;
    m_noiseOffset.y = (rand() % 1000) / 10.0f;
    m_noiseOffset.z = (rand() % 1000) / 10.0f;

	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\cube.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\vertexLightingVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\cloudObjPS.cso");

    
}
void Cloud::Uninit()
{
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void Cloud::Update()
{
	
}


void Cloud::Draw()
{
    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // 定数バッファ更新
    CloudObjParams params;

    params.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    params.Center = { m_position.x, m_position.y, m_position.z, 0 };
    params.Size = { m_scale.x, m_scale.y, m_scale.z, 0 };
    params.Density = m_density; // 濃さ
    params.NoiseOffset = m_noiseOffset;

    Renderer::UpdateCloudBuffer(params); // ※Rendererに定数バッファ更新関数を追加

    // テクスチャ設定
    ID3D11ShaderResourceView* srvs[] = {
        Renderer::GetVolumeNoiseSRV(), // t0: 3Dノイズ
        Renderer::GetDepthSRV()        // t1: 深度バッファ
    };
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 2, srvs);

    // ブレンドステート
    // 通常のアルファブレンドステートをセット
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Renderer::GetDeviceContext()->OMSetBlendState(Renderer::GetBlendState(), blendFactor, 0xFFFFFFFF);



    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
    trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    world = scale * rot * trans;

    Renderer::SetWorldMatrix(world);

    //matrial
    MATERIAL material = {};
    material.Diffuse = {1,1,1,1};
    Renderer::SetMaterial(material);

   
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsNull());
    // 描画実行 (Cubeを描く)
    m_modelRenderer->DrawOitAccumulation();
    //m_modelRenderer->Draw();

    // カリング (表面を描画)
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());

}


void Cloud::DrawImgui()
{
    // IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
    ImGui::PushID(m_No);

    // ヘッダーのラベルだけは番号を表示したいので sprintf が必要
    char headerName[32];
    sprintf(headerName, u8"cloud %d", m_No);

    if (ImGui::CollapsingHeader(headerName))
    {
        // ここは単純に "Position" だけ
        // PushID のおかげで、内部的には "Position/1", "Position/2" のように区別される
        ImGui::DragFloat("m_density", &m_density, 0.1f);
        ImGui::DragFloat3("Position", &m_position.x, 0.1f);
        ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
        
    }

    // 最後に必ず PopID する
    ImGui::PopID();
}
