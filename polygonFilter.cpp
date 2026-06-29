#include "main.h"
#include "polygonFilter.h"
#include "renderer.h"
#include "texture.h"
#include "time.h"

#include "imgui.h"
#include <algorithm>

void polygonFilter::Uninit()
{
	//m_Texture->Release();

	m_PixelShader->Release();
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();

}



void polygonFilter::Init(float x, float y, float width, float height, const char* fileName,bool move)
{
	
	m_move = move;
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(x + width, y, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(x, y + height, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(x + width, y + height, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	////texture yomikomi
	//TexMetadata metadata;
	//ScratchImage image;
	//LoadFromWICFile(L"asset\\texture\\yakei.jpg",WIC_FLAGS_NONE, &metadata, image);
	//CreateShaderResourceView(Renderer::GetDevice(),
	//	image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
	//assert(m_Texture);

	m_Texture = Texture::Load(fileName);
	//m_Texture = Srv;

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\polygonFilterPS.cso");
}

void polygonFilter::Update()
{
}

void polygonFilter::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	Renderer::SetWorldViewProjection2D();

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	trans = XMMatrixTranslation(1.0f, 1.0f, 1.0f);
	world = scale * rot * trans;
	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	if (m_move)
	{

		float f = std::max(0.3f, std::min(sin(Time::TotalTime()), 1.0f));
		f = 1.5f - f;
		material.Diffuse = {f,f,f,1.0f };
	}
	else
		material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);


	//ImGuiIO& io = ImGui::GetIO();
	//ID3D11ShaderResourceView* Srv = (ID3D11ShaderResourceView*)io.Fonts->TexID.GetTexID();
	//Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &Srv);
	

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Renderer::GetDeviceContext()->Draw(4, 0);
}
