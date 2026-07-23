#include "main.h"
#include "renderer.h"
#include <string>
#include "StrTex.h"
#include "manager.h"
#include "camera.h"
#include "texture.h"
#include "scene.h"
#include "PointLight.h"
#include "time.h"
#include "imgui.h"

#include <cstring>

#include <fstream>
#include "json.hpp"

//MSDFでの3D空間テキスト
FontAtlas StrTex::m_FontAtlas;


static FontAtlas LoadMSDFAtlas(const std::string& jsonPath)
{
	FontAtlas atlasData;

	// JSON 読み込み
	std::ifstream ifs(jsonPath);
	nlohmann::json json;
	ifs >> json;

	// atlas サイズ
	atlasData.atlasWidth = json["atlas"]["width"];
	atlasData.atlasHeight = json["atlas"]["height"];

	// メトリクス
	atlasData.lineHeight = json["metrics"]["lineHeight"];
	atlasData.ascender = json["metrics"]["ascender"];
	atlasData.descender = json["metrics"]["descender"];
	for (auto& g : json["glyphs"])
	{
		Glyph glyph;

		glyph.unicode = g["unicode"].get<int>();

		// planeBounds は存在しない可能性あり
		if (g.contains("planeBounds")) {
			auto& pb = g["planeBounds"];
			glyph.pl = pb["left"].get<float>();
			glyph.pb = pb["bottom"].get<float>();
			glyph.pr = pb["right"].get<float>();
			glyph.pt = pb["top"].get<float>();
		}

		// atlasBounds
		if (g.contains("atlasBounds")) {
			auto& ab = g["atlasBounds"];
			float ab_l = ab["left"].get<float>();
			float ab_b = ab["bottom"].get<float>();
			float ab_r = ab["right"].get<float>();
			float ab_t = ab["top"].get<float>();

			glyph.u0 = ab_l / atlasData.atlasWidth;
			glyph.v0 = ab_b / atlasData.atlasHeight;
			glyph.u1 = ab_r / atlasData.atlasWidth;
			glyph.v1 = ab_t / atlasData.atlasHeight;
		}

		// advance
		if (g.contains("advance"))
			glyph.advance = g["advance"].get<float>();
		else
			glyph.advance = 0.0f;

		atlasData.glyphs[glyph.unicode] = glyph;
	}
	

	return atlasData;
}


void StrTex::Init(const char* text, XMFLOAT4 color, bool isrot, Vector3 kaitenziku,float curveRadius)
{
	m_FontAtlas = LoadMSDFAtlas("asset\\A.json");
	//変数に保存すべき？
	text;

	m_curveRadius = curveRadius;
	m_isRot = isrot;
	m_ziku = kaitenziku;
	m_ziku.Normalize();

	m_backColor = color;
	

	float penX = 0.0f;
	int charCount = 0;

	float fixedTop = m_FontAtlas.ascender;
	float fixedBottom = m_FontAtlas.descender;

	for (int i = 0; text[i] != '\0' && charCount < MAX_CHARS; i++)
	{
		//文字サイズによってまだサイズが変わる

		int c = (int)text[i];
		auto it = m_FontAtlas.glyphs.find(c);
		if (it == m_FontAtlas.glyphs.end())
			continue;

		const Glyph& g = it->second;

		float padding = g.advance * 0.2f;

		float quadL = penX;
		float quadR = penX + g.advance + padding * 2.0f;

		float glyphWidth = g.pr - g.pl;
		float glyphL = quadL + padding - g.pl;
		float glyphR = glyphL + glyphWidth;

		float t = fixedTop;
		float b = fixedBottom;

		float u0 = g.u0;
		float u1 = g.u1;
		float v0 = 1.0f - g.v1;
		float v1 = 1.0f - g.v0;

		int v = charCount * 4;

		int i6 = charCount * 6;


		m_Vertex[v + 0] = { { quadL,  t, 0 }, {0,1,0}, {1,1,1,1}, {u0, v0} };
		m_Vertex[v + 1] = { { quadR,  t, 0 }, {0,1,0}, {1,1,1,1}, {u1, v0} };
		m_Vertex[v + 2] = { { quadL,  b, 0 }, {0,1,0}, {1,1,1,1}, {u0, v1} };
		m_Vertex[v + 3] = { { quadR,  b, 0 }, {0,1,0}, {1,1,1,1}, {u1, v1} };

		penX += g.advance + padding * 2.0f;



		// --- Index ---
		m_Index[i6 + 0] = v + 0;
		m_Index[i6 + 1] = v + 1;
		m_Index[i6 + 2] = v + 2;
		m_Index[i6 + 3] = v + 2;
		m_Index[i6 + 4] = v + 1;
		m_Index[i6 + 5] = v + 3;

		charCount++;
	}


	// 文字列全体の幅と高さを確定
	m_TextWidth = penX;
	m_TextHeight = m_FontAtlas.lineHeight;
	m_IndexCount = charCount * 6;



	// VertexBuffer
	D3D11_BUFFER_DESC vb{};
	vb.Usage = D3D11_USAGE_DEFAULT;
	vb.ByteWidth = sizeof(VERTEX_3D) * MAX_CHARS * 4;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = m_Vertex;

	Renderer::GetDevice()->CreateBuffer(&vb, &vsd, &m_VertexBuffer);

	// IndexBuffer
	D3D11_BUFFER_DESC ib{};
	ib.Usage = D3D11_USAGE_DEFAULT;
	ib.ByteWidth = sizeof(UINT) * MAX_CHARS * 6;
	ib.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = m_Index;

	Renderer::GetDevice()->CreateBuffer(&ib, &isd, &m_IndexBuffer);

	m_Texture = Texture::Load("asset\\texture\\A.png");


	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\charTexturePS.cso");
	
}

void StrTex::Uninit()
{
	//m_Texture->Release();

	if(m_PixelShader)m_PixelShader->Release();
	if(m_VertexBuffer)m_VertexBuffer->Release();
	if (m_VertexLayout)m_VertexLayout->Release();
	if (m_VertexShader)m_VertexShader->Release();
	if(m_IndexBuffer)m_IndexBuffer->Release();
}


void StrTex::Update()
{
	//
	if (m_pointLight)
	{
		if (!m_isRot)
		{
			// 看板のY軸の回転角度
			float rad = m_rotation.y;

			// 本来ずらしたい距離
			float offsetX = m_TextWidth * m_scale.x * 0.5f;
			float offsetY = m_TextHeight * m_scale.y * 0.5f;

			// 回転を考慮したXとZのズレを計算 (sinとcosを使います)
			float rotatedOffsetX = offsetX * cosf(rad);
			float rotatedOffsetZ = offsetX * -sinf(rad);

			// Yは回転の影響を受けないのでそのまま
			Vector3 finalPos = m_position + Vector3(rotatedOffsetX, offsetY, rotatedOffsetZ);

			m_pointLight->SetPos(finalPos);
		}
	}


}

void StrTex::Draw()
{

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(
		0, 1, &m_VertexBuffer, &stride, &offset);

	Renderer::GetDeviceContext()->IASetIndexBuffer(
		m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (!m_isRot)
	{//回転無効時


		XMMATRIX world, scale, rot, trans;
		scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
		rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
		world = scale * rot * trans;
		Renderer::SetWorldMatrix(world);
	}
	else
	{

		// 任意軸・公転・外向き・ライト追従

		// 時間
		angle += Time::GamePlayTime();

		// ==============================
		// 任意の回転軸
		// ==============================
		Vector3 a = m_ziku;
		XMVECTOR axis = XMVectorSet(a.x, a.y, a.z, 0.0f);

		if (XMVector3Equal(axis, XMVectorZero()))
		{
			// 軸が設定されていない場合は、とりあえずY軸で回す
			axis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		}
		else
		{
			// 念のため正規化
			axis = XMVector3Normalize(axis);
		}

		// Quaternion 回転
		XMVECTOR q = XMQuaternionRotationAxis(axis, angle);
		XMMATRIX orbitRot = XMMatrixRotationQuaternion(q);

		// ==============================
		// 半径方向（外向き）
		// ==============================
		float orbitRadius = m_radius;

		// ローカル +X を基準
		XMVECTOR localRadiusDir = XMVectorSet(1, 0, 0, 0);

		// 回転後の半径方向
		XMVECTOR forward =
			XMVector3Normalize(
				XMVector3TransformNormal(localRadiusDir, orbitRot)
			);

		// ==============================
		// 安定した回転基底を作る
		// ==============================

		// Right = axis × forward
		XMVECTOR right =
			XMVector3Normalize(
				XMVector3Cross(axis, forward)
			);

		// 左右反転補正
		right = XMVectorNegate(right);

		// Up = forward × right
		XMVECTOR up =
			XMVector3Normalize(
				XMVector3Cross(forward, right)
			);
		up = XMVectorNegate(up);

		// ==============================
		// 回転行列を自作
		// ==============================
		XMMATRIX faceRot;
		faceRot.r[0] = XMVectorSetW(right, 0.0f);
		faceRot.r[1] = XMVectorSetW(up, 0.0f);
		faceRot.r[2] = XMVectorSetW(forward, 0.0f);
		faceRot.r[3] = XMVectorSet(0, 0, 0, 1);

		// ==============================
		// 半径オフセット
		// ==============================
		XMMATRIX orbitOffset =
			XMMatrixTranslationFromVector(forward * orbitRadius);

		XMMATRIX centerTrans = XMMatrixTranslation(
			-m_TextWidth * 0.5f,
			-m_TextHeight * 0.5f,
			0.0f
		);

		// ==============================
		// スケール & 平行移動
		// ==============================
		XMMATRIX scale = XMMatrixScaling(
			m_scale.x, m_scale.y, m_scale.z
		);

		XMMATRIX trans = XMMatrixTranslation(
			m_position.x,
			m_position.y,
			m_position.z
		);

		// ==============================
		// world 行列
		// ==============================
		XMMATRIX world =
			centerTrans*
			scale *
			faceRot *
			orbitOffset *
			trans;

		Renderer::SetWorldMatrix(world);

		// ライト追従処理の修正
		if (m_pointLight)
		{
			XMVECTOR localCenter = XMVectorSet(
				m_TextWidth * 0.5f,
				m_TextHeight * 0.5f,
				0.0f,
				1.0f
			);

			XMVECTOR worldPos = XMVector3TransformCoord(localCenter, world);

			XMFLOAT3 p;
			XMStoreFloat3(&p, worldPos);
			m_pointLight->SetPos(Vector3(p.x, p.y, p.z));
		}
	}



	MATERIAL material{};
	material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
	material.Ambient = { m_backColor };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	//ブルーム色
	BloomColor bc;
	bc.bloomColor = XMFLOAT4(m_litColor.x, m_litColor.y, m_litColor.z, m_litA);
	Renderer::UpdateBloomColor(bc);

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);


	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Renderer::GetDeviceContext()->DrawIndexed(m_IndexCount, 0, 0);

}

void StrTex::LightSwitch(bool on)//これ以前にスケールをセットしていないと現時点はうまくいかない
{
	if (on)
	{
		if (!m_pointLight)
		{
			m_pointLight = Manager::GetScene()->AddGameObject<PointLight>(l_LIGHT);
			m_pointLight->SetPos(m_position);
			m_pointLight->SetIntensity(0.7f * m_scale.x);
			m_pointLight->SetBehavior(PointLight::BehaviorType::None);
			m_pointLight->SetScale({ 6.0f * m_scale.x,6.0f * m_scale.x,6.0f * m_scale.x });

			//これメンバ変数として持たせるカラーにする
			m_pointLight->SetColor(XMFLOAT3(0.0f, 1.0f, 0.0f));
		}
	}
	else
	{
		if (m_pointLight)
		{
			m_pointLight->SetDestroy();
			//多分大丈夫,デストロイ実行前にnullptrにするけど実体自体は残るはず
			m_pointLight = nullptr;
		}
	}
}

void StrTex::SetLitColor(XMFLOAT3 color)
{

	m_litColor = color;
	if(m_pointLight)
	m_pointLight->SetColor(color);
	
}


void StrTex::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	sprintf(headerName, u8"string %d", m_No);

	if (ImGui::CollapsingHeader(headerName))
	{
		// ここは単純に "Position" だけ
		// PushID のおかげで、内部的には "Position/1", "Position/2" のように区別され
		ImGui::DragFloat3("Position", &m_position.x, 0.1f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
		ImGui::DragFloat("litA", &m_litA, 0.1f);


	}

	// 最後に必ず PopID する
	ImGui::PopID();
}
