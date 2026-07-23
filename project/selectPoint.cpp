#include "main.h"
#include "SelectPoint.h"
#include "renderer.h"
#include "texture.h"
//重力UIの決定場所

bool SelectPoint::m_IsDraw = false;
XMFLOAT2 SelectPoint::m_vec;
void SelectPoint::Uninit()
{
	//m_Texture->Release();

	m_PixelShader->Release();
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();

}



void SelectPoint::Init(float x, float y, float width, float height, const char* fileName)
{
	//頂点でサイズとか
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(x- width / 2.0f, y- height / 2.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(x + width / 2.0f, y- height / 2.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(x- width / 2.0f, y + height / 2.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(x + width / 2.0f, y + height / 2.0f, 0.0f);
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

	m_Texture = Texture::Load(fileName);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");


	m_IsDraw = false;
	m_vec = { 0.0f,0.0f };
}

void SelectPoint::Update()
{
	float len;


	len = sqrt(m_vec.x * m_vec.x + m_vec.y * m_vec.y);


	if (len > 50.0f)//中心ととらえる値の大きさ・ゾーン作成時と統一
	{
		XMFLOAT2 norVec;
		norVec.x = m_vec.x / len;
		norVec.y = m_vec.y / len;

		m_position.x = norVec.x * 195;
		m_position.y = norVec.y * 195;
	}
	else
	{
		m_position.x = 0;
		m_position.y = 0;
	}

}

void SelectPoint::Draw()
{
	if (m_IsDraw)
	{
		Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

		Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
		Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

		Renderer::SetWorldViewProjection2D();

		XMMATRIX world, scale, rot, trans;
		scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		rot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		trans = XMMatrixTranslation(m_position.x, m_position.y, 1.0f);
		world = scale * rot * trans;
		Renderer::SetWorldMatrix(world);

		MATERIAL material{};
		material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
		material.TextureEnable = true;
		Renderer::SetMaterial(material);

		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

		Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);


		Renderer::GetDeviceContext()->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		Renderer::GetDeviceContext()->Draw(4, 0);
	}
}
