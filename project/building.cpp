#include "main.h"
#include "manager.h"
#include "Building.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "scene.h"
#include "camera.h"
//足場とするビル

void Building::Init()
{
	//読み込み
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\builTex.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");


	// OBBの情報を更新する
	m_obb.Center = m_position;
	m_obb.Extents = m_scale;
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);
}
void Building::Uninit()
{
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void Building::Update()
{

	// OBBの情報を更新する
	m_obb.Center = m_position;
	m_obb.Extents = m_scale;
	m_obb.Extents.y *= 2.0f;//モデルのサイズと同じ？

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);
}

void Building::Draw()
{
	auto cam = Manager::GetScene()->GetGameObject<Camera>();
	
	if (cam != nullptr)
	{
		//カリング判定
		//半径は適当に大きめに設定（ビルの高さの半分くらい）
		float radius = m_scale.y * 6.0f;

		//posはこの半径で、カメラの視界
		if (!cam->GetFrustum().IsVisible(m_position, radius))
		{
			//視界に入ってないなら、ここで帰る
			return;
		}
	}
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	m_modelRenderer->DrawSetBloom();

}

void Building::DrawShadow()
{//影用のシェーダー群に

	
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	m_modelRenderer->Draw();

}
