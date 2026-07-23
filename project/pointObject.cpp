#include "main.h"
#include "manager.h"
#include "PointObject.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "scene.h"
#include "pointLight.h"
#include "imgui.h"

//スコア加算＋重力ゾーン回数回復



void PointObject::Init()
{
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\ball.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");

	m_scale = { 0.3f ,0.3f ,0.3f };

	m_pointLight = Manager::GetScene()->AddGameObject<PointLight>(l_LIGHT);
	m_pointLight->SetPos(m_position + Vector3{0, m_scale.x * 0.5f, 0});
	m_pointLight->SetIntensity(1.0f * m_scale.x);
	m_pointLight->SetBehavior(PointLight::BehaviorType::None);
	m_pointLight->SetScale({ 6.0f * m_scale.x,6.0f * m_scale.x,6.0f * m_scale.x });

	//これメンバ変数として持たせるカラーにする
	m_pointLight->SetColor(XMFLOAT3(0.1f, 0.1f, 0.5f));

	m_radius = m_scale.x;
	
}
void PointObject::Uninit()
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

void PointObject::Update()
{
	if (m_destroy)
	{
		if (m_pointLight)
		{
			m_pointLight->SetDestroy();
			m_pointLight = nullptr;
		}
	}
	if (m_scale.x != m_radius)
	{
		m_radius = m_scale.x;
		if (m_pointLight)
		{
			m_pointLight->SetIntensity(1.0f * m_scale.x);
			m_pointLight->SetScale({ 6.0f * m_scale.x,6.0f * m_scale.y,6.0f * m_scale.z });
		}
	}
	if (m_pointLight)
	{
		m_pointLight->SetPos(m_position);
	}
}

void PointObject::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	if (m_isHighlight)
	{
		BloomColor bc;
		bc.bloomColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
		Renderer::UpdateBloomColor(bc);
	}
	else
	{
		BloomColor bc;
		bc.bloomColor = XMFLOAT4(0.1f, 0.1f, 0.5f, 0.5f);
		Renderer::UpdateBloomColor(bc);
	}

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	if (!m_destroy)
	m_modelRenderer->Draw();

}

void PointObject::DrawShadow()
{//影用のシェーダー群に
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	if (!m_destroy)
	m_modelRenderer->Draw();

}


void PointObject::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	if(m_isCreateObj)
		sprintf(headerName, u8"PointObj(Create) %d", m_No);
	else
		sprintf(headerName, u8"PointObj %d", m_No);

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

		ImGui::Checkbox("isSave", &m_isSaveObj);
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) m_isHighlight = true;
	}

	// 最後に必ず PopID する
	ImGui::PopID();
}
