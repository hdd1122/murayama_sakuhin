#include "main.h"
#include "manager.h"
#include "GroundBox.h"
#include "renderer.h"
#include "modelRenderer.h"

#include "scene.h"
#include "imgui.h"

#include "Time.h"

//BOXの足場



void GroundBox::Init()
{

	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\cube3.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\groundVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\groundPS.cso");

	// OBBの情報を更新する
	m_obb.Center = m_position;
	m_obb.Extents = m_scale;

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);

}
void GroundBox::Uninit()
{
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void GroundBox::Update()
{
	// OBBの情報を更新する
	m_obb.Center = m_position;
	m_obb.Extents = m_scale;

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);

}

void GroundBox::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


	XMMATRIX scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	m_world = scale * rot * trans;

	Renderer::SetWorldMatrix(m_world);

	m_modelRenderer->Draw();

}

void GroundBox::DrawShadow()
{
	//影用のシェーダー群に
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


	XMMATRIX scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	m_world = scale * rot * trans;

	Renderer::SetWorldMatrix(m_world);
	m_modelRenderer->Draw();

}

void GroundBox::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	if (m_isCreateObj)
		sprintf(headerName, u8"BOX(Create) %d", m_No);
	else
		sprintf(headerName, u8"BOX %d", m_No);

	if (ImGui::CollapsingHeader(headerName))
	{
		// ここは単純に "Position" だけ
		// PushID のおかげで、内部的には "Position/1", "Position/2" のように区別される
		ImGui::DragFloat3("Position", &m_position.x,0.1f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
		ImGui::Checkbox("isSave", &m_isSaveObj);
	}

	// 最後に必ず PopID する
	ImGui::PopID();
}
