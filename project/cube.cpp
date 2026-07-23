#include "main.h"
#include "manager.h"
#include "Cube.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"

//タイトル用BOX



void Cube::Init()
{
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\cube.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");


	
}
void Cube::Uninit()
{
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();

}

void Cube::Update()
{


}

void Cube::Draw()
{
	//共通設定
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = { m_color.x,m_color.y,m_color.z,1.0f };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);
	m_modelRenderer->DrawOitAccumulation();

}

void Cube::DrawShadow()
{//影用のシェーダー群に
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


	//Renderer::SetWorldViewProjection2D();

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);




	m_modelRenderer->Draw();

}
