#include "main.h"
#include "renderer.h"
#include <string>
#include "StrUI.h"
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

//MSDFでのUI

FontAtlas StrUI::m_FontAtlas;


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


void StrUI::Init(int maxChars)
{
	m_MaxChars = maxChars;

	// 1. フォント読み込み
	if (m_FontAtlas.glyphs.empty()) {
		m_FontAtlas = LoadMSDFAtlas("asset\\A.json"); // static関数にするか名前を変える！
	}

	// 2. Vertex Buffer
	// 文字が変わるたびに書き換えるので DYNAMIC にする
	D3D11_BUFFER_DESC vb{};
	vb.ByteWidth = sizeof(VERTEX_3D) * m_MaxChars * 4; // 4頂点/文字
	vb.Usage = D3D11_USAGE_DYNAMIC;        // GPUに「書き換えるよ」と伝える
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPUから書き込み許可

	// nullptr で作成
	Renderer::GetDevice()->CreateBuffer(&vb, nullptr, &m_VertexBuffer);

	// 3. Index Buffer
	// インデックスの並び順は文字が変わっても絶対変わらないので
	// ここで最大数分作って固定してしまう
	std::vector<UINT> indices(m_MaxChars * 6);
	for (int i = 0; i < m_MaxChars; i++) {
		int v = i * 4;
		int idx = i * 6;
		indices[idx + 0] = v + 0;
		indices[idx + 1] = v + 1;
		indices[idx + 2] = v + 2;
		indices[idx + 3] = v + 2;
		indices[idx + 4] = v + 1;
		indices[idx + 5] = v + 3;
	}

	D3D11_BUFFER_DESC ib{};
	ib.ByteWidth = sizeof(UINT) * indices.size();
	ib.Usage = D3D11_USAGE_IMMUTABLE; // 書き換えない
	ib.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = indices.data();
	Renderer::GetDevice()->CreateBuffer(&ib, &isd, &m_IndexBuffer);

	// 4. テクスチャ・シェーダー
	m_Texture = Texture::Load("asset\\texture\\A.png");


	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\stringUIPS.cso");
}

// text更新処理
void StrUI::SetText(const std::string& text)
{
	// 文字列が変わっていなければ何もしない
	if (m_CurrentText == text) return;
	m_CurrentText = text;

	// 文字数が最大を超えないように
	int len = (int)text.length();
	if (len > m_MaxChars) len = m_MaxChars;
	m_CurrentCharCount = len;

	// --- 頂点データの計算 (CPU側) ---
	std::vector<VERTEX_3D> vertices;
	vertices.reserve(len * 4);

	float penX = 0.0f;
	float fixedTop = m_FontAtlas.ascender;
	float fixedBottom = m_FontAtlas.descender;

	for (int i = 0; i < len; i++)
	{
		int c = (int)text[i];
		if (m_FontAtlas.glyphs.find(c) == m_FontAtlas.glyphs.end()) continue;

		const Glyph& g = m_FontAtlas.glyphs[c];
		float padding = g.advance * 0.2f; // パディングなどはお好みで

		float u0 = g.u0; float u1 = g.u1;
		float v0 = 1.0f - g.v1; float v1 = 1.0f - g.v0;

		float quadL = penX;
		float quadR = penX + g.advance + padding * 2.0f;

		// 頂点作成 //座標系の違いがあるため-fixedTopなどにしている
		VERTEX_3D v0_tl = { { quadL, -fixedTop, 0 },    {0,1,0}, m_Color, {u0, v0} };
		VERTEX_3D v1_tr = { { quadR, -fixedTop, 0 },    {0,1,0}, m_Color, {u1, v0} };
		VERTEX_3D v2_bl = { { quadL, -fixedBottom, 0 }, {0,1,0}, m_Color, {u0, v1} };
		VERTEX_3D v3_br = { { quadR, -fixedBottom, 0 }, {0,1,0}, m_Color, {u1, v1} };

		vertices.push_back(v0_tl);
		vertices.push_back(v1_tr);
		vertices.push_back(v2_bl);
		vertices.push_back(v3_br);

		penX += g.advance + padding * 2.0f;
	}

	// --- GPUへ転送 (Map / Unmap) ---
	D3D11_MAPPED_SUBRESOURCE ms;
	// MAP_WRITE_DISCARD: 「前のデータは捨てていいから、新しいメモリ領域をくれ」という指示。
	// これによりGPUが前の描画でバッファを使っていても待たされずに済む
	HRESULT hr = Renderer::GetDeviceContext()->Map(
		m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);

	if (SUCCEEDED(hr))
	{
		// サイズが0より大きい時だけコピーする
		if (!vertices.empty()) {
			memcpy(ms.pData, vertices.data(), sizeof(VERTEX_3D) * vertices.size());
		}
		Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}

void StrUI::Uninit()
{
	//m_Texture->Release();

	if(m_PixelShader)m_PixelShader->Release();
	if(m_VertexBuffer)m_VertexBuffer->Release();
	if (m_VertexLayout)m_VertexLayout->Release();
	if (m_VertexShader)m_VertexShader->Release();
	if(m_IndexBuffer)m_IndexBuffer->Release();
}


void StrUI::Update()
{
	
}

void StrUI::Draw()
{
	if (m_CurrentCharCount == 0) return;

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// 頂点バッファセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Renderer::SetWorldViewProjection2D();

	XMMATRIX world, scale, rot, trans;
	trans = XMMatrixTranslation(m_position.x, m_position.y, 0.0f);
	scale = XMMatrixScaling(m_scale.x, m_scale.y, 1.0f);
	rot = XMMatrixIdentity();
	world = scale * rot * trans;
	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
	material.Ambient = { 0,0,0,0 };//これを文字の背景に使ってる
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	//ブルーム色(いらないけど一応0セット)
	BloomColor bc;
	bc.bloomColor = XMFLOAT4(0,0,0,0);
	Renderer::UpdateBloomColor(bc);

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);


	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 描画 (現在の文字数 * 6インデックス だけ描画)
	Renderer::GetDeviceContext()->DrawIndexed(m_CurrentCharCount * 6, 0, 0);

}



void StrUI::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	sprintf(headerName, u8"stringUI %d", m_No);

	if (ImGui::CollapsingHeader(headerName))
	{
		// ここは単純に "Position" だけ
		// PushID のおかげで、内部的には "Position/1", "Position/2" のように区別され
		ImGui::DragFloat3("Position", &m_position.x, 0.1f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
		
	}

	// 最後に必ず PopID する
	ImGui::PopID();
}
