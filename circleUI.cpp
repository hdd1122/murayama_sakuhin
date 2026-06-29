#include "main.h"
#include "manager.h"
#include "circleUI.h"
#include "renderer.h"
#include "texture.h"
#include "selectPoint.h"
#include "camera.h"
#include "scene.h"

//重力ゾーンの方向を設定するためのリング状UI

//
XMFLOAT4 HSVtoRGB_C(float h, float s, float v, float a = 1.0f)
{
	float c = v * s;
	float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
	float m = v - c;

	float r, g, b;
	if (h < 60) { r = c; g = x; b = 0; }
	else if (h < 120) { r = x; g = c; b = 0; }
	else if (h < 180) { r = 0; g = c; b = x; }
	else if (h < 240) { r = 0; g = x; b = c; }
	else if (h < 300) { r = x; g = 0; b = c; }
	else { r = c; g = 0; b = x; }

	return { r + m, g + m, b + m, a };
}

// 世界座標の3Dベクトルを「絶対的な色」に変換する
XMFLOAT4 GetWorldColorFromVector_C(Vector3 dir)
{
		dir.Normalize();

		// XY平面での角度を計算（右(X+)を0度、上(Y+)を90度とする）
		float hue = atan2f(dir.y, dir.x) * 180.0f / XM_PI;
		if (hue < 0) hue += 360.0f;

		// Z方向（奥行き）の強さに応じて彩度を調整
		// Z方向に突き抜けるほど色が白に近づく（色の跳ねを抑える）
		float depth = fabsf(dir.z);
		float s = 1.0f - (depth * 0.5f);
		float v = 1.0f;

		return HSVtoRGB_C(hue, s, v, 1.0f);
	
}

bool CircleUI::m_IsDraw = false;
void CircleUI::Uninit()
{
	//m_Texture->Release();

	m_PixelShader->Release();
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();


}


void CircleUI::Init(float x, float y, float innerRadius, float outerRadius, int segments)
{
	m_posX = x; m_posY = y;
	m_innerRadius = innerRadius; m_quterRadius = outerRadius;
	m_segments = segments;

	int numVertices = (segments + 1) * 2;
	m_vertexCount = numVertices;

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC; // 動的に変更可能にする
	bd.ByteWidth = sizeof(VERTEX_3D) * numVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPUからの書き込み許可

	// 初期データは空でも良いが、サイズ確保のために作成
	Renderer::GetDevice()->CreateBuffer(&bd, NULL, &m_VertexBuffer);
	// シェーダーの設定
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_IsDraw = false;
	m_SelectPoint = Manager::GetScene()->AddGameObject<SelectPoint>(l_UI);
	m_SelectPoint->Init(ScreenSize::ScreenWidth / 2.0f, ScreenSize::ScreenHeight / 2.0f, 150, 150, "asset\\texture\\testSelectPoint.png");

}

void CircleUI::Update()
{
	if (m_IsDraw)
	{
		Camera* cam = Manager::GetScene()->GetGameObject<Camera>();
		Vector3 camRight = cam->GetRight();
		Vector3 camUp = cam->GetUp();

		std::vector<VERTEX_3D> vertices(m_vertexCount);

		for (int i = 0; i <= m_segments; ++i)
		{
			float ratio = (float)i / (float)m_segments;
			float angleRad = ratio * XM_2PI;

			// 数学的な座標系（右がcos, 上がsin）
			float vx = cosf(angleRad);
			float vy = sinf(angleRad);

			// 世界座標でのベクトルをシミュレート
			// vyが正のとき、世界の上を指すように設定
			Vector3 segmentWorldDir = (camRight * vx) + (camUp * vy);

			XMFLOAT4 color = GetWorldColorFromVector_C(segmentWorldDir);

			// UI表示用の座標計算
			// スクリーン座標では「下」が正なので、表示位置は vy を反転させる
			float screenX = m_posX + vx * m_quterRadius;
			float screenY = m_posY - vy * m_quterRadius; // ここでマイナスして表示位置を上に持っていく

			int idxOut = i * 2;
			vertices[idxOut].Position = XMFLOAT3(screenX, screenY, 0.0f);
			vertices[idxOut].Diffuse = color;

			float screenInX = m_posX + vx * m_innerRadius;
			float screenInY = m_posY - vy * m_innerRadius;
			int idxIn = i * 2 + 1;
			vertices[idxIn].Position = XMFLOAT3(screenInX, screenInY, 0.0f);
			vertices[idxIn].Diffuse = color;
		}

		// GPU上のバッファを更新
		D3D11_MAPPED_SUBRESOURCE ms;
		HRESULT hr = Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (SUCCEEDED(hr)) {
			memcpy(ms.pData, vertices.data(), sizeof(VERTEX_3D) * m_vertexCount);
			Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
		}
	}
}

void CircleUI::Draw()
{
	if (m_IsDraw)
	{
		//基本描画処理
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
		material.Diffuse = { 1.0f,1.0f,1.0f,0.8f };//半透明
		material.TextureEnable = false;//テクスチャ無し
		Renderer::SetMaterial(material);

		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
		if (m_Texture) {
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
		}

		Renderer::GetDeviceContext()->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		Renderer::GetDeviceContext()->Draw(m_vertexCount, 0);
	}
}

void CircleUI::SetIsDraw(bool draw)
{
	m_IsDraw = draw;
	SelectPoint::SetIsDraw(draw);
}
