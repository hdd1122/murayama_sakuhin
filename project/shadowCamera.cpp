#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "ShadowCamera.h"
#include "player.h"
#include "input.h"
#include "scene.h"
#include "imgui.h"

#include <algorithm> 

//シャドウマップ用カメラ

void ShadowCamera::Init()
{
	//これdirectionはposとtarのベクトルで計算するべき？
	m_position = { 1.0f, 50.0f, 0.0f };
	m_Target = { 0.0f, 0.0f, 0.0f };

	Vector3 direction = m_Target - m_position;	//これ一応正規化してないベクトル//posの位置が近いけど
												//directionのみ使うなら関係ない？

	m_light.Enable = true;
	m_light.CastsShadows = true;
	//m_light.Direction = XMFLOAT4(0.5f, -1.0f, 0.5f, 0.0f);
	m_light.Direction = XMFLOAT4(-0.1f, -1.0f, 0.0f, 0.0f);
	//
	m_light.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.3f, 1.0f);
	m_light.Ambient = XMFLOAT4(0.12f, 0.13f, 0.25f, 1.0f);
	
}
void ShadowCamera::Uninit()
{


}

void ShadowCamera::Update()
{
	// ライト
	
	//m_light.enable = true;
	//m_light.direction = xmfloat4(0.5f, -1.0f, 0.5f, 0.0f);
	//m_light.ambient = xmfloat4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_light.diffuse = xmfloat4(1.5f, 1.5f, 1.5f, 1.0f);

	Vector3 direction = m_Target - m_position;	//これ一応正規化してないベクトル//posの位置が近いけど
	//directionのみ使うなら関係ない？
	
	direction.Normalize();
	//これカメラのアングルが真上とか真下になるのは防ぐべき
	m_light.Direction = XMFLOAT4(direction.x, direction.y, direction.z, 0.0f);

	Renderer::SetLight(m_light);
}

void ShadowCamera::Draw()
{


	
}
void ShadowCamera::SetMatrix()
{
	m_projection = XMMatrixOrthographicLH(20.0f, 20.0f, 1.0f, 100.0f);

	// ビュー行列を生成
	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	m_view = XMMatrixLookAtLH(
		XMLoadFloat3((XMFLOAT3*)&m_position),
		XMLoadFloat3((XMFLOAT3*)&m_Target),
		up
	);
}

void ShadowCamera::DrawImgui()
{
	
	if (ImGui::CollapsingHeader(u8"DLight")) {
		// ▼ ヘッダーが開いている時だけ、詳細情報を表示する
		ImGui::ColorEdit3("dffuse", &m_light.Ambient.x);
		ImGui::DragFloat3("pos", &m_position.x,0.3f);
	}
	
}
