#include "main.h"
#include "manager.h"
#include "sky.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "scene.h"

//スカイドーム

void Sky::Init()
{
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\sky.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\skyVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\skyPS.cso");
}
void Sky::Uninit()
{
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void Sky::Update()
{
	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();
	m_position = camera->GetPos();

	

}

void Sky::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	m_modelRenderer->Draw();

}
