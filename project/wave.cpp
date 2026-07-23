#include "main.h"
#include "renderer.h"
#include "wave.h"
#include "texture.h"

//メッシュフィールド拡張の波
void Wave::Init()
{

	// 頂点バッファ生成
	{
		for (int x = 0; x < 21; x++)
		{
			for (int z = 0; z < 21; z++)
			{
				m_Vertex[x][z].Position =
					XMFLOAT3((x - 10) * 5.0f, 0.0f, (z - 10) * -5.0f);
				m_Vertex[x][z].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				m_Vertex[x][z].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				m_Vertex[x][z].TexCoord = XMFLOAT2(x, z);
			}
		}


		//法線ベクトル算出



		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX_3D) * 21 * 21;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = m_Vertex;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);
	}




	// インデックスバッファ生成
	{
		unsigned int index[((21 + 1) * 2) * 20 - 2];

		int i = 0;
		for (int x = 0; x < 20; x++)
		{
			for (int z = 0; z < 21; z++)
			{
				index[i] = x * 21 + z;
				i++;

				index[i] = (x + 1) * 21 + z;
				i++;
			}

			if (x == 19)
				break;

			index[i] = (x + 1) * 21 + 20;
			i++;

			index[i] = (x + 1) * 21;
			i++;
		}

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * ((22 * 2) * 20 - 2);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = index;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer);
	}





	m_Texture = Texture::Load("asset\\texture\\wave.jpg");


	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\oitAccumPS.cso");

}


void Wave::Uninit()
{

	m_VertexBuffer->Release();
	//m_Texture->Release();
	//m_EnvTexture->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();


}


void Wave::Update()
{

	for (int x = 0; x < 21; x++)
	{
		for (int z = 0; z < 21; z++)
		{
			float dx = m_Vertex[x][z].Position.x - m_Vertex[0][0].Position.x;
			float dz = m_Vertex[x][z].Position.z - m_Vertex[0][0].Position.z;
			float length = sqrtf(dx * dx + dz * dz);
			m_Vertex[x][z].Position.y = 0.5f * sinf(m_Time * -1.0f + length * 0.5f);
		}
	}

	m_Time += 1.0f / 60.0f;



	//頂点データ書き換え
	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	memcpy(vertex, m_Vertex, sizeof(VERTEX_3D) * 21 * 21);

	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);

}



void Wave::Draw()
{
	// 入力レイアウト設定
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	// シェーダ設定
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// ワールドマトリクス設定
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;
	Renderer::SetWorldMatrix(world);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(
		m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// マテリアル設定
	MATERIAL material{};
	//ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	//Renderer::GetDeviceContext()->PSSetShaderResources(1, 1, &m_EnvTexture);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画
	//Renderer::GetDeviceContext()->Draw(21 * 21, 0);
	Renderer::GetDeviceContext()->DrawIndexed((22 * 2) * 20 - 2, 0, 0);


   // ラスタライザーステートを表面カリングに切り替える
	Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsFront());

	MATERIAL materialInner = material;
	materialInner.Diffuse.x = material.Diffuse.x * 0.25f;
	materialInner.Diffuse.y = material.Diffuse.y * 0.25f;
	materialInner.Diffuse.z = material.Diffuse.z * 0.25f;
	materialInner.Diffuse.w = material.Diffuse.w * 0.25f;
	Renderer::SetMaterial(materialInner);

	Renderer::GetDeviceContext()->DrawIndexed((22 * 2) * 20 - 2, 0, 0);

	Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());
	
}


