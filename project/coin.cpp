#include "main.h"
#include "manager.h"
#include "Coin.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "scene.h"
#include "pointLight.h"
#include "time.h"
#include "imgui.h"

//スコア加算オブジェクト



void Coin::Init()
{
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\coin.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");

	m_pointLight = Manager::GetScene()->AddGameObject<PointLight>(l_LIGHT);
	m_pointLight->SetPos(m_position + Vector3{0, m_scale.x * 0.5f, 0});
	m_pointLight->SetIntensity(1.0f * m_scale.x);
	m_pointLight->SetBehavior(PointLight::BehaviorType::None);
	m_pointLight->SetScale({ 6.0f * m_scale.x,6.0f * m_scale.x,6.0f * m_scale.x });

	//これメンバ変数として持たせるカラーにする
	m_pointLight->SetColor(XMFLOAT3(0.5f, 0.5f, 0.1f));
	
}
void Coin::Uninit()
{
	if (m_pointLight)
	{
		m_pointLight->SetDestroy();
		m_pointLight = nullptr;
	}
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void Coin::Update()
{
	if (m_destroy)
	{//セーブの為実際に削除はしない
		if (m_pointLight)
		{
			m_pointLight->SetDestroy();
			m_pointLight = nullptr;
		}
	}
	if (m_scale.x != m_radius)
	{//スケール変更された場合
		m_radius = m_scale.x;
		m_pointLight->SetIntensity(1.0f * m_scale.x);
		m_pointLight->SetScale({ 6.0f * m_scale.x,6.0f * m_scale.y,6.0f * m_scale.z });
	}
	if (m_pointLight)
	{//追尾
		m_pointLight->SetPos(m_position + Vector3{ 0, m_scale.x * 0.5f, 0 });
	}
	m_rotation.y += 5 * Time::GamePlayTime();
	if(m_rotation.y > 10000)
		m_rotation.y = 0;
}

void Coin::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	//imguiの選択状況で分岐
	//発光オブジェクトはこれでいいけど、
	if (m_isHighlight)
	{
		BloomColor bc;
		bc.bloomColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
		Renderer::UpdateBloomColor(bc);
	}
	else
	{
		BloomColor bc;
		bc.bloomColor = XMFLOAT4(0.5f, 0.5f, 0.1f, 0.5f);
		Renderer::UpdateBloomColor(bc);
	}



	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y * 0.5f, m_scale.z);//モデル自体を変更すべきだけど問題はない
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x + XM_PI * 0.5f, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	if(!m_destroy)
	m_modelRenderer->Draw();

}

void Coin::DrawShadow()
{//影用のシェーダー群に
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y * 0.5f, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x + XM_PI * 0.5f, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	if (!m_destroy)
	m_modelRenderer->Draw();

}

void Coin::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	if (m_isCreateObj)
		sprintf(headerName, u8"coin(Create) %d", m_No);
	else
		sprintf(headerName, u8"coin %d", m_No);


	// 毎フレーム最初はフラグを折っておく
	m_isHighlight = false;

	// ヘッダーを描画し、その状態を取得
	bool isOpen = ImGui::CollapsingHeader(headerName);

	// ヘッダー自体にマウスが乗っている、またはクリックされているか判定
	if (ImGui::IsItemHovered() || ImGui::IsItemActive())
	{
		m_isHighlight = true;
	}

	if (isOpen)
	{
		ImGui::DragFloat3("Position", &m_position.x, 0.1f);
		// DragFloat3 の直後に呼び出すことで、この Position のUIの状態を判定
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) m_isHighlight = true;

		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) m_isHighlight = true;

		ImGui::Checkbox("isSave?", &m_isSaveObj);
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) m_isHighlight = true;
	}



	// 最後に必ず PopID する
	ImGui::PopID();
}
