#include "main.h"
#include "manager.h"
#include "realityFragment.h"
#include "renderer.h"
#include "texture.h"
#include "scene.h"

//現在の空間パーティクル


void RealityFragment::Uninit()
{
	
	if (m_PixelShader) m_PixelShader->Release();
	if (m_VertexShader) m_VertexShader->Release();
	if (m_VertexLayout) m_VertexLayout->Release();

	if (m_ComputeShader) m_ComputeShader->Release();

	if (m_ParticleBufferA) m_ParticleBufferA->Release();
	if (m_ParticleBufferB) m_ParticleBufferB->Release();
	if (m_ParticleSRVA) m_ParticleSRVA->Release();
	if (m_ParticleSRVB) m_ParticleSRVB->Release();
	if (m_ParticleUAVA) m_ParticleUAVA->Release();
	if (m_ParticleUAVB) m_ParticleUAVB->Release();

	if (m_QuadVertexBuffer) m_QuadVertexBuffer->Release();
	if (m_QuadIndexBuffer) m_QuadIndexBuffer->Release();
	
}



void RealityFragment::Init(UINT maxParticles, const char* fileName)
{

	m_MaxParticles = maxParticles;

	// パーティクルの初期データをCPU側で生成
	std::vector<Particle> initialParticles(m_MaxParticles);
	for (UINT i = 0; i < m_MaxParticles; ++i)
	{
		// ここでパーティクルの初期位置や速度をランダムに設定する
		initialParticles[i].Position = {
		Renderer::RandomFloat(-100.0f, 100.0f),
		Renderer::RandomFloat(0.0f, 20.0f),
		Renderer::RandomFloat(-100.0f, 100.0f)
		};

		initialParticles[i].Velocity = { 0.0f, 1.0f, 0.0f };
		initialParticles[i].Life = 1.0f + (rand() / (float)RAND_MAX); // 寿命を少しばらつかせる
		initialParticles[i].Age = 0.0f;
		initialParticles[i].IsActive = 0;
	}
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = initialParticles.data();

	// GPUが読み書きするためのバッファ(StructuredBuffer)を2つ作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Particle) * m_MaxParticles;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(Particle);

	// バッファAとBを、同じ設定で初期データを使って作成
	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &sd, &m_ParticleBufferA);
	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &sd, &m_ParticleBufferB);

	// 各バッファに対応するSRV(読み取り用)とUAV(書き込み用)を作成
	// SRVの設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements = m_MaxParticles;
	Renderer::GetDevice()->CreateShaderResourceView(m_ParticleBufferA, &srvDesc, &m_ParticleSRVA);
	Renderer::GetDevice()->CreateShaderResourceView(m_ParticleBufferB, &srvDesc, &m_ParticleSRVB);

	// UAVの設定
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = m_MaxParticles;
	Renderer::GetDevice()->CreateUnorderedAccessView(m_ParticleBufferA, &uavDesc, &m_ParticleUAVA);
	Renderer::GetDevice()->CreateUnorderedAccessView(m_ParticleBufferB, &uavDesc, &m_ParticleUAVB);


	// インスタンシング用の板ポリゴンを作成
	VERTEX_3D vertices[] = { // 1枚の板の頂点
		{ {-0.5f, 0.5f, 0.0f}, {0,0,1}, {1,1,1,1}, {0,0} },
		{ { 0.5f, 0.5f, 0.0f}, {0,0,1}, {1,1,1,1}, {1,0} },
		{ {-0.5f,-0.5f, 0.0f}, {0,0,1}, {1,1,1,1}, {0,1} },
		{ { 0.5f,-0.5f, 0.0f}, {0,0,1}, {1,1,1,1}, {1,1} }
	};
	// 頂点バッファの作成 
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.Usage = D3D11_USAGE_DEFAULT; // 最も一般的な使用法
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0; // CPUからのアクセスは不要
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sdVertex = {};
	sdVertex.pSysMem = vertices;
	sdVertex.SysMemPitch = 0;
	sdVertex.SysMemSlicePitch = 0;

	// バッファを作成
	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&vbDesc, &sdVertex, &m_QuadVertexBuffer);
	if (FAILED(hr)) {
		
	}


	// 2つの三角形を構成するためのインデックスデータ (6インデックス)
	WORD indices[] = { 0, 1, 2, 1, 3, 2 };

	// --- インデックスバッファの作成 ---
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(indices);
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sdIndex = {};
	sdIndex.pSysMem = indices;
	sdIndex.SysMemPitch = 0;
	sdIndex.SysMemSlicePitch = 0;

	// バッファを作成
	hr = Renderer::GetDevice()->CreateBuffer(&ibDesc, &sdIndex, &m_QuadIndexBuffer);
	if (FAILED(hr)) {
		// エラー処理
	}


	m_Texture = Texture::Load(fileName);



	Renderer::CreateParticleVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\fragmentVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\fragmentPS.cso");

	Renderer::CreateComputeShader(&m_ComputeShader,
		"shader\\fragmentCS.cso");

}

void RealityFragment::Update()
{
	auto* context = Renderer::GetDeviceContext();

	//コンピュートシェーダーをセット
	context->CSSetShader(m_ComputeShader, nullptr, 0);

	//入力と出力を設定
	//フレームごとに入力と出力のバッファを入れ替える
	ID3D11ShaderResourceView* srv = m_PingPong ? m_ParticleSRVB : m_ParticleSRVA;
	ID3D11UnorderedAccessView* uav = m_PingPong ? m_ParticleUAVA : m_ParticleUAVB;
	context->CSSetShaderResources(0, 1, &srv);
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	//コンピュートシェーダーを実行
	//GPUに「パーティクルの数 ÷ スレッドグループのサイズ」分の計算を並列で実行させる
	if (m_MaxParticles > 0) {
		context->Dispatch((m_MaxParticles + 255) / 256, 1, 1);
	}

	//バインドを解除 (他のシェーダーがこれらのリソースを使えるようにするため)
	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	//次のフレームのために入れ替える
	m_PingPong = !m_PingPong;
}

void RealityFragment::Draw()
{
	auto* context = Renderer::GetDeviceContext();


	//描画用のシェーダーとインプットレイアウトをセット
	context->IASetInputLayout(m_VertexLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0); // ジオメトリシェーダーは使わない

	//2種類の頂点バッファをセット
	//スロット0: 全パーティクル共通の「板ポリゴン」
	//スロット1: Updateで更新されたばかりの「パーティクルごとのデータ」
	ID3D11Buffer* buffers[] = { m_QuadVertexBuffer, m_PingPong ? m_ParticleBufferA : m_ParticleBufferB };
	UINT strides[] = { sizeof(VERTEX_3D), sizeof(Particle) };
	UINT offsets[] = { 0, 0 };
	context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	context->IASetIndexBuffer(m_QuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//パーティクルのテクスチャをセット
	context->PSSetShaderResources(0, 1, &m_Texture);

	//インスタンシング描画を呼び出す
	//6個のインデックスを持つ板ポリゴン」を、「m_MaxParticles」の数だけ複製して描画する
	context->DrawIndexedInstanced(6, m_MaxParticles, 0, 0, 0);
	
}

