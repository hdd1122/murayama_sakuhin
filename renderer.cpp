
#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"

#include <io.h>

#ifdef _DEBUG
#define DX11_SET_NAME(obj, name) \
    if ((obj) != nullptr) { \
        (obj)->SetPrivateData( \
            WKPDID_D3DDebugObjectName, \
            (UINT)strlen(name), \
            name \
        ); \
    }
#else
#define DX11_SET_NAME(obj, name)
#endif

//staticメンバ変数など
D3D_FEATURE_LEVEL       Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device*           Renderer::m_Device{};
ID3D11DeviceContext*    Renderer::m_DeviceContext{};
IDXGISwapChain*         Renderer::m_SwapChain{};
ID3D11RenderTargetView* Renderer::m_RenderTargetView{};
ID3D11DepthStencilView* Renderer::m_DepthStencilView{};

ID3D11Buffer*			Renderer::m_WorldBuffer{};
ID3D11Buffer*			Renderer::m_ViewBuffer{};
ID3D11Buffer*			Renderer::m_ProjectionBuffer{};
ID3D11Buffer*			Renderer::m_MaterialBuffer{};
ID3D11Buffer*			Renderer::m_LightBuffer{};
ID3D11Buffer*			Renderer::m_ZoneBuffer{};
ID3D11Buffer*			Renderer::m_blurParamsBuffer{};
ID3D11Buffer*			Renderer::m_compositeParamsBuffer{};
ID3D11Buffer*			Renderer::m_bloomColorBuffer{};
ID3D11Buffer*			Renderer::m_raymarchingParamsBuffer{};
ID3D11Buffer*			Renderer::m_csmParamsBuffer{};
ID3D11Buffer*			Renderer::m_pointLightBuffer{};
ID3D11Buffer*			Renderer::m_cloudBuffer{};


ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};

ID3D11BlendState*		Renderer::m_BlendState{};
ID3D11BlendState*		Renderer::m_BlendStateATC{};
ID3D11BlendState*		Renderer::m_additiveBlendState{};
ID3D11BlendState*		Renderer::m_blendStatePMA{};

ID3D11BlendState*		Renderer::m_oitAccumBlendState{};

ID3D11Texture2D*		Renderer::m_accumTexture{};
ID3D11Texture2D*		Renderer::m_revealTexture{};
ID3D11RenderTargetView* Renderer::m_accumRTV{};
ID3D11RenderTargetView* Renderer::m_revealRTV{};
ID3D11ShaderResourceView* Renderer::m_accumSRV{};
ID3D11ShaderResourceView* Renderer::m_revealSRV{};

ID3D11DepthStencilState* Renderer::m_depthStateReadOnly{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisabled{};

ID3D11VertexShader*		 Renderer::m_CompositeVS{};
ID3D11PixelShader*		 Renderer::m_CompositePS{};
ID3D11InputLayout*		 Renderer::m_VertexLayout{};

ID3D11RasterizerState*	 Renderer::m_rsBack;
ID3D11RasterizerState*	 Renderer::m_rsFront;
ID3D11RasterizerState*	 Renderer::m_rsNone;
ID3D11SamplerState*		 Renderer::m_samplerState;
ID3D11SamplerState*		 Renderer::m_linearSampler;

ID3D11Texture2D*			Renderer::m_bloomContributionTexture;
ID3D11RenderTargetView*		Renderer::m_bloomContributionRTV;
ID3D11ShaderResourceView*	Renderer::m_bloomContributionSRV;

ID3D11Texture2D*			Renderer::m_downSampleTexture_Half;
ID3D11RenderTargetView*		Renderer::m_downSampleRTV_Half;
ID3D11ShaderResourceView*	Renderer::m_downSampleSRV_Half;

ID3D11Texture2D*			Renderer::m_downSampleTexture_Quarter;
ID3D11RenderTargetView*		Renderer::m_downSampleRTV_Quarter;
ID3D11ShaderResourceView*	Renderer::m_downSampleSRV_Quarter;
ID3D11VertexShader* Renderer::m_postEffectVS;
ID3D11PixelShader*	Renderer::m_downsamplePS;
ID3D11PixelShader*	Renderer::m_blurPS;
ID3D11PixelShader*  Renderer::m_bloomCompositePS;

ID3D11Texture2D*			Renderer::m_tempTexture_Quarter;
ID3D11RenderTargetView*		Renderer::m_tempRTV_Quarter;
ID3D11ShaderResourceView*	Renderer::m_tempSRV_Quarter;

ID3D11Texture2D*			Renderer::m_sceneColorTexture;
ID3D11RenderTargetView*		Renderer::m_sceneColorRTV;
ID3D11ShaderResourceView*	Renderer::m_sceneColorSRV;

ID3D11PixelShader* Renderer::m_copyPS;
ID3D11PixelShader* Renderer::m_bloomUpsamplePS;

ID3D11VertexShader* Renderer::m_unlitVS = nullptr;
ID3D11PixelShader* Renderer::m_unlitPS = nullptr;
ID3D11InputLayout* Renderer::m_unlitLayout = nullptr;
ID3D11RasterizerState* Renderer::m_rsWireframe = nullptr;

ID3D11VertexShader* Renderer::m_rayTestVS = nullptr;
ID3D11PixelShader* Renderer:: m_rayTestPS = nullptr;



ID3D11Texture2D* Renderer::m_downSampleTextures[bloomLevel];
ID3D11RenderTargetView* Renderer::m_downSampleRTVs[bloomLevel];
ID3D11ShaderResourceView* Renderer::m_downSampleSRVs[bloomLevel];

ID3D11Texture2D* Renderer::m_tempTextures[bloomLevel];
ID3D11RenderTargetView* Renderer::m_tempRTVs[bloomLevel];
ID3D11ShaderResourceView* Renderer::m_tempSRVs[bloomLevel];

ID3D11ShaderResourceView* Renderer::m_DepthSRV;

ID3D11Texture2D* Renderer::			m_sceneNormalTexture;
ID3D11RenderTargetView* Renderer::	m_sceneNormalRTV;
ID3D11ShaderResourceView* Renderer::m_sceneNormalSRV;

ID3D11Texture2D* Renderer::			m_sceneWorldPosTexture;
ID3D11RenderTargetView* Renderer::	m_sceneWorldPosRTV;
ID3D11ShaderResourceView* Renderer::m_sceneWorldPosSRV;

ID3D11Texture2D* Renderer::			m_sceneLitColorTexture;
ID3D11RenderTargetView* Renderer::	m_sceneLitColorRTV;
ID3D11ShaderResourceView* Renderer::m_sceneLitColorSRV;

ID3D11Texture2D* Renderer::m_shadowMapTexture;
ID3D11DepthStencilView* Renderer::m_shadowMapDSV[CASCADE_COUNT];
ID3D11ShaderResourceView* Renderer::m_shadowMapSRV;
ID3D11RasterizerState* Renderer::m_shadowRS;
ID3D11SamplerState* Renderer::m_shadowSampler;

ID3D11PixelShader* Renderer::m_gLightingPS;
ID3D11PixelShader* Renderer::m_rayDepthPS;
ID3D11PixelShader* Renderer::m_raymShadowPS;
ID3D11VertexShader* Renderer::m_shadowVS;


ID3D11DepthStencilState* Renderer::m_depthStateLight;

ID3D11PixelShader* Renderer::m_pointLightPS;
ID3D11VertexShader* Renderer::m_pointLightVS;
ID3D11InputLayout* Renderer::m_litLayout;

//
ID3D11Texture3D* Renderer::m_noiseTexture = nullptr;
ID3D11ShaderResourceView* Renderer::m_noiseSRV = nullptr;

ID3D11PixelShader* Renderer::m_cloudPS;

ID3D11SamplerState* Renderer::m_bloomSampler = nullptr;

ID3D11Buffer* Renderer::m_FullscreenVB = nullptr;
ID3D11Buffer* Renderer::m_rFullscreenVB = nullptr;

RECT Renderer::m_windowedRect; // 元のウィンドウサイズ保存用
bool Renderer::m_isFullscreen = false;

ID3D11ShaderResourceView* Renderer::m_DummyTextureSRV;


ID3D11Buffer* Renderer::cloudCB = nullptr;

std::vector<PointLightParams> Renderer::m_lightList;
ID3D11Buffer* Renderer::m_lightInstBuffer = nullptr;
ID3D11ShaderResourceView* Renderer::m_lightInstSRV = nullptr;
ModelRenderer* Renderer::m_commonSphereModel = nullptr;

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
void Renderer::CheckD3D11Leaks()
{
#ifdef _DEBUG
	typedef HRESULT(WINAPI* GetDebugInterfaceFunc)(REFIID, void**);

	HMODULE hDxgiDebug = LoadLibraryA("dxgidebug.dll");
	if (!hDxgiDebug) return;

	auto getDebug = (GetDebugInterfaceFunc)GetProcAddress(
		hDxgiDebug,
		"DXGIGetDebugInterface"
	);

	if (getDebug)
	{
		IDXGIDebug* debug = nullptr;
		if (SUCCEEDED(getDebug(IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(
				DXGI_DEBUG_ALL,
				DXGI_DEBUG_RLO_DETAIL
			);
			debug->Release();
		}
	}

	FreeLibrary(hDxgiDebug);

	// --- D3D11 ---
	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&d3dDebug))))
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		d3dDebug->Release();
	}
#endif
}




void Renderer::Uninit()
{
	//解放処理
	if (m_DeviceContext)
	{
		ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
		m_DeviceContext->PSSetShaderResources(
			0,
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
			nullSRVs
		);

		m_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		m_DeviceContext->ClearState();
		m_DeviceContext->Flush();
	}
	// 最初
	if (m_DeviceContext)
	{
		m_DeviceContext->ClearState();
		m_DeviceContext->Flush();
	}

	delete m_commonSphereModel;
	m_commonSphereModel = nullptr;
	ModelRenderer::UnloadAll();

	//ライト
	m_lightList.clear();
	// ↓ その後に Release 群
	// テクスチャはそれのSRVとかより後に解放すべきらしい
	
	// 定数バッファ
	SAFE_RELEASE(m_WorldBuffer);
	SAFE_RELEASE(m_ViewBuffer);
	SAFE_RELEASE(m_ProjectionBuffer);
	SAFE_RELEASE(m_MaterialBuffer);
	SAFE_RELEASE(m_LightBuffer);
	SAFE_RELEASE(m_ZoneBuffer);
	SAFE_RELEASE(m_blurParamsBuffer);
	SAFE_RELEASE(m_compositeParamsBuffer);
	SAFE_RELEASE(m_raymarchingParamsBuffer);
	SAFE_RELEASE(m_pointLightBuffer);
	SAFE_RELEASE(m_cloudBuffer);
	SAFE_RELEASE(m_csmParamsBuffer);
	SAFE_RELEASE(cloudCB);
	SAFE_RELEASE(m_lightInstBuffer);
	
	SAFE_RELEASE(m_FullscreenVB);
	SAFE_RELEASE(m_rFullscreenVB);

	// デプス・ステンシルステート
	SAFE_RELEASE(m_DepthStateEnable);
	SAFE_RELEASE(m_DepthStateDisable);
	SAFE_RELEASE(m_depthStateReadOnly);
	SAFE_RELEASE(m_DepthStateDisabled);
	SAFE_RELEASE(m_oitAccumBlendState);

	// ブレンドステート
	SAFE_RELEASE(m_BlendState);
	SAFE_RELEASE(m_BlendStateATC);
	SAFE_RELEASE(m_additiveBlendState);
	SAFE_RELEASE(m_blendStatePMA);

	// OIT用リソース
	SAFE_RELEASE(m_accumSRV);
	SAFE_RELEASE(m_revealSRV);
	SAFE_RELEASE(m_accumRTV);
	SAFE_RELEASE(m_revealRTV);
	SAFE_RELEASE(m_accumTexture);
	SAFE_RELEASE(m_revealTexture);

	// ブルーム用リソース
	SAFE_RELEASE(m_bloomContributionSRV);
	SAFE_RELEASE(m_bloomContributionRTV);
	SAFE_RELEASE(m_bloomContributionTexture);

	// その他
	SAFE_RELEASE(m_CompositeVS);
	SAFE_RELEASE(m_CompositePS);
	SAFE_RELEASE(m_VertexLayout);
	SAFE_RELEASE(m_rsBack);
	SAFE_RELEASE(m_rsFront);
	SAFE_RELEASE(m_rsNone);
	SAFE_RELEASE(m_samplerState);
	SAFE_RELEASE(m_bloomSampler);
	SAFE_RELEASE(m_linearSampler);

	SAFE_RELEASE(m_downSampleSRV_Half);
	SAFE_RELEASE(m_downSampleRTV_Half);
	SAFE_RELEASE(m_downSampleTexture_Half);

	SAFE_RELEASE(m_downSampleSRV_Quarter);
	SAFE_RELEASE(m_downSampleRTV_Quarter);
	SAFE_RELEASE(m_downSampleTexture_Quarter);

	SAFE_RELEASE(m_postEffectVS);
	SAFE_RELEASE(m_downsamplePS);
	SAFE_RELEASE(m_blurPS);
	SAFE_RELEASE(m_bloomCompositePS);

	SAFE_RELEASE(m_tempRTV_Quarter);
	SAFE_RELEASE(m_tempSRV_Quarter);
	SAFE_RELEASE(m_tempTexture_Quarter);

	SAFE_RELEASE(m_sceneColorRTV);
	SAFE_RELEASE(m_sceneColorSRV);
	SAFE_RELEASE(m_sceneColorTexture);
	SAFE_RELEASE(m_copyPS);

	SAFE_RELEASE(m_unlitVS);
	SAFE_RELEASE(m_unlitPS);
	SAFE_RELEASE(m_rayTestVS);
	SAFE_RELEASE(m_rayTestPS);
	SAFE_RELEASE(m_rayDepthPS);
	SAFE_RELEASE(m_gLightingPS);
	SAFE_RELEASE(m_raymShadowPS);
	SAFE_RELEASE(m_shadowVS);
	SAFE_RELEASE(m_pointLightVS);
	SAFE_RELEASE(m_pointLightPS);
	SAFE_RELEASE(m_bloomUpsamplePS);

	SAFE_RELEASE(m_unlitLayout);
	SAFE_RELEASE(m_rsWireframe);

	SAFE_RELEASE(m_cloudPS);

	//
	SAFE_RELEASE(m_depthStateLight);
	SAFE_RELEASE(m_litLayout);

	//
	SAFE_RELEASE(m_noiseSRV);
	SAFE_RELEASE(m_noiseTexture);

	// g-buffer kannrenn
	SAFE_RELEASE(m_sceneNormalRTV);
	SAFE_RELEASE(m_sceneNormalSRV);
	SAFE_RELEASE(m_sceneNormalTexture);

	SAFE_RELEASE(m_sceneWorldPosRTV);
	SAFE_RELEASE(m_sceneWorldPosSRV);
	SAFE_RELEASE(m_sceneWorldPosTexture);

	SAFE_RELEASE(m_sceneLitColorRTV);
	SAFE_RELEASE(m_sceneLitColorSRV);
	SAFE_RELEASE(m_sceneLitColorTexture);


	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		SAFE_RELEASE(m_shadowMapDSV[i]);
	}
	SAFE_RELEASE(m_shadowMapSRV);

	SAFE_RELEASE(m_shadowMapTexture);
	SAFE_RELEASE(m_shadowRS);
	SAFE_RELEASE(m_shadowSampler);

	SAFE_RELEASE(m_DummyTextureSRV);
	


	for (int i = 0; i < bloomLevel; i++)
	{

		SAFE_RELEASE(m_downSampleRTVs[i]);
		SAFE_RELEASE(m_downSampleSRVs[i]);
		SAFE_RELEASE(m_downSampleTextures[i]);

		SAFE_RELEASE(m_tempRTVs[i]);
		SAFE_RELEASE(m_tempSRVs[i]);
		SAFE_RELEASE(m_tempTextures[i]);
	}
	
	SAFE_RELEASE(m_bloomColorBuffer);

	SAFE_RELEASE(m_DepthSRV);

	// pointLightkannrenn
	SAFE_RELEASE(m_lightInstSRV);

	if (m_SwapChain)
	{
		// 強制的にウィンドウモードに戻す（フルスクリーンのまま破棄すると落ちるため）
		m_SwapChain->SetFullscreenState(FALSE, nullptr);
	}
	// DirectX基本オブジェクト
	SAFE_RELEASE(m_DepthStencilView);
	SAFE_RELEASE(m_RenderTargetView);
	SAFE_RELEASE(m_SwapChain);

	if (m_DeviceContext)
	{
		// コンテキストをクリアしてから消すのが作法
		m_DeviceContext->ClearState();
		m_DeviceContext->Flush();
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}

	
	// 全て消し終わった後に、リークチェックを呼ぶ
#ifdef _DEBUG
	CheckD3D11Leaks();
#endif
	// Deviceもここで消す
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}


}

void Renderer::Init()
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = ScreenSize::ScreenWidth;
	swapChainDesc.BufferDesc.Height = ScreenSize::ScreenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = GetWindow();
	swapChainDesc.SampleDesc.Count = 1;//moto
	//swapChainDesc.SampleDesc.Count = 4;//msaaはディファードではやめとくべき
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		deviceFlags,        //
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		&m_FeatureLevel,
		&m_DeviceContext
	);

	assert(SUCCEEDED(hr));

	// DXGIファクトリーを取得して、自動Alt+Enterを無効化する
	IDXGIDevice* pDXGIDevice = nullptr;
	if (SUCCEEDED(m_Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice)))
	{
		IDXGIAdapter* pDXGIAdapter = nullptr;
		if (SUCCEEDED(pDXGIDevice->GetAdapter(&pDXGIAdapter)))
		{
			IDXGIFactory* pIDXGIFactory = nullptr;
			if (SUCCEEDED(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory)))
			{
				pIDXGIFactory->MakeWindowAssociation(GetWindow(), DXGI_MWA_NO_ALT_ENTER);
				pIDXGIFactory->Release();
			}
			pDXGIAdapter->Release();
		}
		pDXGIDevice->Release();
	}

	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget{};
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	m_Device->CreateRenderTargetView(renderTarget, NULL, &m_RenderTargetView);
	renderTarget->Release();



	// 半透明用レンダーターゲット
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = ScreenSize::ScreenWidth;
	texDesc.Height = ScreenSize::ScreenHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//texDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	m_Device->CreateTexture2D(&texDesc, nullptr, &m_accumTexture);
	m_Device->CreateTexture2D(&texDesc, nullptr, &m_revealTexture);

	// 各テクスチャに対応するRTVとSRVを作成
	m_Device->CreateRenderTargetView(m_accumTexture, nullptr, &m_accumRTV);
	m_Device->CreateShaderResourceView(m_accumTexture, nullptr, &m_accumSRV);
	m_Device->CreateRenderTargetView(m_revealTexture, nullptr, &m_revealRTV);
	m_Device->CreateShaderResourceView(m_revealTexture, nullptr, &m_revealSRV);

	//worldPos
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//texDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	m_Device->CreateTexture2D(&texDesc, nullptr, &m_sceneWorldPosTexture);

	// 各テクスチャに対応するRTVとSRVを作成
	m_Device->CreateRenderTargetView(m_sceneWorldPosTexture, nullptr, &m_sceneWorldPosRTV);
	m_Device->CreateShaderResourceView(m_sceneWorldPosTexture, nullptr, &m_sceneWorldPosSRV);

	//法線
	texDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	m_Device->CreateTexture2D(&texDesc, nullptr, &m_sceneNormalTexture);

	// 各テクスチャに対応するRTVとSRVを作成
	m_Device->CreateRenderTargetView(m_sceneNormalTexture, nullptr, &m_sceneNormalRTV);
	m_Device->CreateShaderResourceView(m_sceneNormalTexture, nullptr, &m_sceneNormalSRV);
	//カラーはもともと使ってたやつを変更するかも

	{
		HRESULT hr;

		// --- 1. テクスチャ本体の作成 ---
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = ScreenSize::ScreenWidth;
		texDesc.Height = ScreenSize::ScreenHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		// フォーマット
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		// 描画先(RT)としても、読み取り元(SR)としても使えるように設定
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		hr = m_Device->CreateTexture2D(&texDesc, nullptr, &m_bloomContributionTexture);
		assert(SUCCEEDED(hr));




		// --- 2. 描画用ビュー(RTV)の作成 ---
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		hr = m_Device->CreateRenderTargetView(m_bloomContributionTexture, &rtvDesc, &m_bloomContributionRTV);
		assert(SUCCEEDED(hr));



		// --- 3. 読み取り用ビュー(SRV)の作成 ---
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = m_Device->CreateShaderResourceView(m_bloomContributionTexture, &srvDesc, &m_bloomContributionSRV);
		assert(SUCCEEDED(hr));

		// --- 1. テクスチャ本体の作成 ---
		D3D11_TEXTURE2D_DESC SceneTexDesc = {};
		SceneTexDesc.Width = ScreenSize::ScreenWidth;
		SceneTexDesc.Height = ScreenSize::ScreenHeight;
		SceneTexDesc.MipLevels = 1;
		SceneTexDesc.ArraySize = 1;
		// フォーマット
		SceneTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//これDXGI_FORMAT_R8G8B8A8_UNORM_SRGBにするかも
		SceneTexDesc.SampleDesc.Count = 1;
		SceneTexDesc.Usage = D3D11_USAGE_DEFAULT;
		// 描画先(RT)としても、読み取り元(SR)としても使えるように設定
		SceneTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		SceneTexDesc.CPUAccessFlags = 0;
		SceneTexDesc.MiscFlags = 0;



		hr = m_Device->CreateTexture2D(&SceneTexDesc, nullptr, &m_sceneColorTexture);
		assert(SUCCEEDED(hr));
		hr = m_Device->CreateRenderTargetView(m_sceneColorTexture, nullptr, &m_sceneColorRTV);
		assert(SUCCEEDED(hr));
		hr = m_Device->CreateShaderResourceView(m_sceneColorTexture, nullptr, &m_sceneColorSRV);
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateTexture2D(&SceneTexDesc, nullptr, &m_sceneLitColorTexture);
		assert(SUCCEEDED(hr));
		hr = m_Device->CreateRenderTargetView(m_sceneLitColorTexture, nullptr, &m_sceneLitColorRTV);
		assert(SUCCEEDED(hr));
		hr = m_Device->CreateShaderResourceView(m_sceneLitColorTexture, nullptr, &m_sceneLitColorSRV);
		assert(SUCCEEDED(hr));


	}

	// --- ダウンサンプル用レンダーターゲットの作成 ---
	{
		// 半分サイズ (Half Resolution)
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = ScreenSize::ScreenWidth / 2;
		texDesc.Height = ScreenSize::ScreenHeight / 2;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDRフォーマット
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		m_Device->CreateTexture2D(&texDesc, nullptr, &m_downSampleTexture_Half);
		m_Device->CreateRenderTargetView(m_downSampleTexture_Half, nullptr, &m_downSampleRTV_Half);
		m_Device->CreateShaderResourceView(m_downSampleTexture_Half, nullptr, &m_downSampleSRV_Half);


		// 4分の1サイズ (Quarter Resolution)
		texDesc.Width = ScreenSize::ScreenWidth / 4;
		texDesc.Height = ScreenSize::ScreenHeight / 4;

		m_Device->CreateTexture2D(&texDesc, nullptr, &m_downSampleTexture_Quarter);
		m_Device->CreateRenderTargetView(m_downSampleTexture_Quarter, nullptr, &m_downSampleRTV_Quarter);
		m_Device->CreateShaderResourceView(m_downSampleTexture_Quarter, nullptr, &m_downSampleSRV_Quarter);

		texDesc.Width = ScreenSize::ScreenWidth / 4;
		texDesc.Height = ScreenSize::ScreenHeight / 4;

		hr = m_Device->CreateTexture2D(&texDesc, nullptr, &m_tempTexture_Quarter);
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateRenderTargetView(m_tempTexture_Quarter, nullptr, &m_tempRTV_Quarter);
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateShaderResourceView(m_tempTexture_Quarter, nullptr, &m_tempSRV_Quarter);
		assert(SUCCEEDED(hr));


		//new
		{

			D3D11_TEXTURE2D_DESC texDesc = {};

			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDRフォーマット
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			for (int i = 0; i < bloomLevel; i++)
			{
				int divisor = 2 << i; // 2, 4, 8, 16....
				texDesc.Width = ScreenSize::ScreenWidth / divisor;
				texDesc.Height = ScreenSize::ScreenHeight / divisor;

				m_Device->CreateTexture2D(&texDesc, nullptr, &m_downSampleTextures[i]);
				m_Device->CreateRenderTargetView(m_downSampleTextures[i], nullptr, &m_downSampleRTVs[i]);
				m_Device->CreateShaderResourceView(m_downSampleTextures[i], nullptr, &m_downSampleSRVs[i]);

				m_Device->CreateTexture2D(&texDesc, nullptr, &m_tempTextures[i]);
				m_Device->CreateRenderTargetView(m_tempTextures[i], nullptr, &m_tempRTVs[i]);
				m_Device->CreateShaderResourceView(m_tempTextures[i], nullptr, &m_tempSRVs[i]);
			}


		}


	}

	{
	// ダミー黒テクスチャの生成

	// 真っ黒なピクセルデータを用意 (RGBA = 0, 0, 0, 0)
	uint32_t blackPixel = 0x00000000;

	// 初期化データの設定
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &blackPixel;
	initData.SysMemPitch = sizeof(uint32_t); // 1行あたりのバイト数(1pxなので4バイト)

	// テクスチャの設定 (1x1ピクセル)
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 一般的なカラーフォーマット
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;       // 書き換えないのでIMMUTABLEで高速化
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// テクスチャ作成
	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = GetDevice()->CreateTexture2D(&desc, &initData, &pTexture);

	if (SUCCEEDED(hr))
	{
		// シェーダーリソースビュー(SRV)の作成
		hr = GetDevice()->CreateShaderResourceView(pTexture, nullptr, &m_DummyTextureSRV);

		pTexture->Release();
	}

	}

	{
		// --- デプスステンシルバッファ作成 ---
		ID3D11Texture2D* depthStencilTexture = nullptr;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = ScreenSize::ScreenWidth;
		texDesc.Height = ScreenSize::ScreenHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; //TYPELESSフォーマット
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //SRVのフラグを追加
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		m_Device->CreateTexture2D(&texDesc, NULL, &depthStencilTexture);


		// --- デプスステンシルビュー(DSV)作成 ---
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //DSV用のフォーマット
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;//MS無しが元　多分うまく動作しない
		m_Device->CreateDepthStencilView(depthStencilTexture, &dsvDesc, &m_DepthStencilView);


		// --- 深度シェーダーリソースビュー(SRV)作成 ---
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; //SRV用のフォーマット
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_Device->CreateShaderResourceView(depthStencilTexture, &srvDesc, &m_DepthSRV);

		// 作成に使ったテクスチャポインタは解放
		depthStencilTexture->Release();
	}

	//m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);


	{
		
		// --- デプスステンシルバッファ作成 ---
		m_shadowMapTexture = nullptr;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = SHADOW_MAP_SIZE;
		texDesc.Height = SHADOW_MAP_SIZE;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = CASCADE_COUNT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS; //TYPELESSフォーマット
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //SRVのフラグを追加
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		m_Device->CreateTexture2D(&texDesc, NULL, &m_shadowMapTexture);


		// --- デプスステンシルビュー(DSV)作成 ---
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //DSV用のフォーマット
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.ArraySize = 1;

		for (int i = 0; i < CASCADE_COUNT; i++)
		{
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			m_Device->CreateDepthStencilView(m_shadowMapTexture, &dsvDesc, &m_shadowMapDSV[i]);
		}


		// --- 深度シェーダーリソースビュー(SRV)作成 ---
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT; //SRV用のフォーマット
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = CASCADE_COUNT;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		m_Device->CreateShaderResourceView(m_shadowMapTexture, &srvDesc, &m_shadowMapSRV);

		
	}



	// ビューポート設定
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)ScreenSize::ScreenWidth;
	viewport.Height = (FLOAT)ScreenSize::ScreenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	//m_DeviceContext->RSSetViewports( 1, &viewport );



	// ラスタライザステート設定
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID; 
	
	rasterizerDesc.CullMode = D3D11_CULL_BACK; 
	rasterizerDesc.DepthClipEnable = TRUE; 
	rasterizerDesc.MultisampleEnable = FALSE; 

	m_Device->CreateRasterizerState( &rasterizerDesc, &m_rsBack );

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_rsFront);
	
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_rsNone);


	//m_DeviceContext->RSSetState( rs );

	{
		// --- ワイヤーフレーム用ラスタライザーステートの作成 ---
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME; // ワイヤーフレームに設定
		rasterizerDesc.CullMode = D3D11_CULL_NONE;      // 裏表両方描画
		rasterizerDesc.DepthClipEnable = TRUE;

		m_Device->CreateRasterizerState(&rasterizerDesc, &m_rsWireframe);
	}

	{//シャドウ用
		
		D3D11_RASTERIZER_DESC rsDesc = {};
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_BACK; // 通常は裏面カリングでOK
		rsDesc.FrontCounterClockwise = FALSE;

		// --- ここからがシャドウマップ用の設定 ---

		// デプスバイアスを有効にする
		rsDesc.DepthBias = 1000; // 固定値のオフセット。まずは数千単位で試す。

		// 傾斜スケールデプスバイアスを有効にする
		// ポリゴンの傾きが急なほど、オフセットを大きくする
		rsDesc.SlopeScaledDepthBias = 1.0f; // 傾斜に応じたオフセット。まずは1.0前後で試す。

		rsDesc.DepthBiasClamp = 0.0f; // バイアスの最大値
		rsDesc.DepthClipEnable = TRUE;
		rsDesc.ScissorEnable = FALSE;
		rsDesc.MultisampleEnable = FALSE;
		rsDesc.AntialiasedLineEnable = FALSE;

		hr = m_Device->CreateRasterizerState(&rsDesc, &m_shadowRS);
		if (FAILED(hr)) {
			//エラー処理
		}
	}

	// ブレンドステート設定
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_Device->CreateBlendState( &blendDesc, &m_BlendState );

	blendDesc.AlphaToCoverageEnable = TRUE;
	m_Device->CreateBlendState( &blendDesc, &m_BlendStateATC );





	// OIT蓄積パス用 または ポイントライト用 の加算ブレンドステート
	D3D11_BLEND_DESC additiveBlendDesc = {};
	additiveBlendDesc.RenderTarget[0].BlendEnable = TRUE;

	// 足す色（ライトの色）を 100% 使う
	additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;

	// 元の色（背景）も 100% 残す
	// (ここが ZERO や INV_SRC_ALPHA だと背景が消えます)
	additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	// アルファチャンネルの合成設定（RGBと同じでOK）
	additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_Device->CreateBlendState(&additiveBlendDesc, &m_additiveBlendState);



	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	//m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff );

	D3D11_BLEND_DESC pmaBlendDesc = {};
	pmaBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	pmaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE; // ソースは1
	pmaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // デストは(1 - SrcA)
	pmaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pmaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	pmaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	pmaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	pmaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_Device->CreateBlendState(&pmaBlendDesc, &m_blendStatePMA);

	{

		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = true; //RTごとに違う設定をするフラグ

		// --- RT0 (Accumulation): 加算合成 ---
		// 色と重みを単純に足し込んでいく
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		// --- RT1 (Revealage): 乗算合成 (Zero, InvSrcColor) ---
		// 「背景がどれだけ見えるか(1.0)」から、描画するオブジェクトのアルファ分だけ削る
		// 計算式: Dest * (1 - Src)
		blendDesc.RenderTarget[1].BlendEnable = true;
		blendDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_COLOR; // 1.0 - Src
		blendDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; // 1.0 - SrcAlpha
		blendDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED; // R成分だけでOK

		GetDevice()->CreateBlendState(&blendDesc, &m_oitAccumBlendState);
	}


	// デプスステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateEnable );//深度有効ステート

	//depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateDisable );//深度無効ステート

	// ステンシルステート
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //書き込みを無効化
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	m_Device->CreateDepthStencilState(&dsDesc, &m_depthStateReadOnly);

	D3D11_DEPTH_STENCIL_DESC dsDescDisabled = {};
	dsDescDisabled.DepthEnable = FALSE; // ★深度テスト自体を無効化
	dsDescDisabled.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState(&dsDescDisabled, &m_DepthStateDisabled);

	//ライトボリューム用デプスステート
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		// 深度テストは行う
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		dsDesc.StencilEnable = FALSE;
		m_Device->CreateDepthStencilState(&dsDesc, &m_depthStateLight);
	}


	//m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, NULL);



	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;//元4
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//ID3D11SamplerState* samplerState{};
	m_Device->CreateSamplerState( &samplerDesc, &m_samplerState );

	//m_DeviceContext->PSSetSamplers( 0, 1, &samplerState );
	{

		D3D11_SAMPLER_DESC sampDesc = {};

		// --- ここからがシャドウマップ用の設定 ---

		// 比較フィルタ (PCF) を指定
		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		// MIN_MAG_LINEAR : 近くの4点を線形補間（バイリニア）してPCFを滑らかにする
		// MIP_POINT      : ミップマップは使わない

		// 比較関数を指定
		// シェーダーが渡す深度値 <= テクスチャの深度値 ならパス(1.0)
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

		// アドレッシングモード（UV範囲外の扱い）
		// シャドウマップの範囲外(0.0～1.0の外)を参照した場合、
		// BorderColor の値(1.0)を返すようにする
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		// ボーダーカラー
		// 範囲外は「影ではない(1.0)」として扱うため、白(1.0, 1.0, 1.0, 1.0)を指定
		sampDesc.BorderColor[0] = 1.0f;
		sampDesc.BorderColor[1] = 1.0f;
		sampDesc.BorderColor[2] = 1.0f;
		sampDesc.BorderColor[3] = 1.0f;
		// --- ここまで ---

		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 1; // 比較フィルタでは異方性フィルタリングは使われない
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_Device->CreateSamplerState(&sampDesc, &m_shadowSampler);
		if (FAILED(hr)) {
			// エラー処理
		}
	}

	{
		D3D11_SAMPLER_DESC bloomSamplerDesc = {};
		bloomSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		bloomSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		bloomSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		bloomSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		bloomSamplerDesc.BorderColor[0] = 0.0f;
		bloomSamplerDesc.BorderColor[1] = 0.0f;
		bloomSamplerDesc.BorderColor[2] = 0.0f;
		bloomSamplerDesc.BorderColor[3] = 0.0f;
		bloomSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	
		m_Device->CreateSamplerState(&bloomSamplerDesc, &m_bloomSampler);

	}
	

	{
		// UI / MSDF用 (Linear + Clamp)サンプラー
		D3D11_SAMPLER_DESC linearDesc = {};

		// 線形補間 (Linear)
		// MSDFを綺麗に出すにはバイリニア補間が必須(Pointだとガビガビになる)
		linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		// アドレッシング: クランプ (Clamp)
		// テクスチャの端(0.0～1.0)を超えたら、端の色を引き伸ばします。
		// これを WRAP にしていると、隣の文字や反対側のノイズを拾ってしまい、
		// さっきの画像のような「縞模様」や「ノイズ」の原因になります。
		linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		// 3. その他
		linearDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // 比較しない
		linearDesc.MinLOD = 0;
		linearDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_Device->CreateSamplerState(&linearDesc, &m_linearSampler);
	}

	// 定数バッファ生成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_WorldBuffer );
	
	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ViewBuffer );
	
	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ProjectionBuffer );
	
	bufferDesc.ByteWidth = sizeof(MATERIAL);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_MaterialBuffer );


	bufferDesc.ByteWidth = sizeof(LIGHT);
	m_Device->CreateBuffer( &bufferDesc, NULL, &m_LightBuffer );
	

	bufferDesc.ByteWidth = sizeof(ZONE);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_ZoneBuffer);
	
	bufferDesc.ByteWidth = sizeof(BlurParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_blurParamsBuffer);
	
	bufferDesc.ByteWidth = sizeof(CompositeParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_compositeParamsBuffer);

	bufferDesc.ByteWidth = sizeof(BloomColor);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_bloomColorBuffer);

	bufferDesc.ByteWidth = sizeof(RaymarchingParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_raymarchingParamsBuffer);

	bufferDesc.ByteWidth = sizeof(CsmParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_csmParamsBuffer);

	bufferDesc.ByteWidth = sizeof(PointLightParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_pointLightBuffer);

	bufferDesc.ByteWidth = sizeof(CloudObjParams);
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_cloudBuffer);




	// ライト初期化
	LIGHT light{};
	light.Enable = true;
	light.Direction = XMFLOAT4(0.5f, -1.0f, 0.5f, 0.0f);
	light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	SetLight(light);



	// マテリアル初期化
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);


	BloomColor bc;
	bc.bloomColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	UpdateBloomColor(bc);

	//シェーダー関連
	Renderer::CreateVertexShader(&m_CompositeVS, &m_VertexLayout, "shader\\compositeVS.cso");
	Renderer::CreatePixelShader(&m_CompositePS, "shader\\compositePS.cso");

	Renderer::CreateVertexShader(&m_postEffectVS, nullptr, "shader/posteffectVS.cso"); // レイアウト不要
	Renderer::CreatePixelShader(&m_downsamplePS, "shader/downsamplePS.cso");
	Renderer::CreatePixelShader(&m_blurPS, "shader/blurPS.cso");
	Renderer::CreatePixelShader(&m_bloomCompositePS, "shader/bloomCompositePS.cso");
	Renderer::CreatePixelShader(&m_copyPS, "shader/copyPS.cso");

	Renderer::CreateVertexShader(&m_unlitVS, &m_unlitLayout, "shader/unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_unlitPS, "shader/unlitTexturePS.cso");

	Renderer::CreateVertexShader(&m_rayTestVS, nullptr, "shader/rayTestVS.cso");
	Renderer::CreatePixelShader(&m_rayTestPS, "shader/rayTestPS.cso");

	Renderer::CreatePixelShader(&m_gLightingPS, "shader/g_lightingPS.cso");
	Renderer::CreatePixelShader(&m_rayDepthPS, "shader/rayDepthPS.cso");
	Renderer::CreatePixelShader(&m_raymShadowPS, "shader/raymShadowPS.cso");

	Renderer::CreateVertexShader(&m_shadowVS, nullptr, "shader/shadowVS.cso");


	Renderer::CreateVertexShader(&m_pointLightVS, &m_litLayout,
		"shader\\pointLightVS.cso");
	Renderer::CreatePixelShader(&m_pointLightPS,
		"shader\\pointLightPS.cso");

	Renderer::CreatePixelShader(&m_cloudPS, "shader\\cloudPS.cso");
	Renderer::CreatePixelShader(&m_bloomUpsamplePS, "shader\\bloomUpsamplePS.cso");


	//3dtex
	CreateVolumeNoiseTexture();

	//ポイントライト用初期化（インスタンシング）
	InitPointLightSystem();
}



//もはやあまり機能していないが初期化的な
void Renderer::Begin()
{
	// 描画先と深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	// ビューポートを設定
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)ScreenSize::ScreenWidth;
	viewport.Height = (FLOAT)ScreenSize::ScreenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports(1, &viewport);

	// 基本的な描画ステートをセット
	m_DeviceContext->RSSetState(m_rsBack);
	m_DeviceContext->PSSetSamplers(0, 1, &m_samplerState);

	//カリングもデフォに
	Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());

	// 全てのオブジェクトで共通して使う定数バッファをセット
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_WorldBuffer);
	m_DeviceContext->VSSetConstantBuffers(1, 1, &m_ViewBuffer);
	m_DeviceContext->VSSetConstantBuffers(2, 1, &m_ProjectionBuffer);
	m_DeviceContext->VSSetConstantBuffers(3, 1, &m_MaterialBuffer);
	m_DeviceContext->VSSetConstantBuffers(4, 1, &m_LightBuffer);
	m_DeviceContext->PSSetConstantBuffers(3, 1, &m_MaterialBuffer);
	m_DeviceContext->PSSetConstantBuffers(4, 1, &m_LightBuffer);

	// 5. レンダーターゲットをクリア
	float clearColor[4] = { 0.5f, 0.5f, 0.8f, 1.0f };
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	

}



void Renderer::End()
{
	m_SwapChain->Present( 0, 0 );
}




void Renderer::SetDepthEnable( bool Enable )
{
	if( Enable )
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );
	else
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateDisable, NULL );

}



void Renderer::SetATCEnable( bool Enable )
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
		m_DeviceContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);

}

void Renderer::SetWorldViewProjection2D()
{
	SetWorldMatrix(XMMatrixIdentity());
	SetViewMatrix(XMMatrixIdentity());

	XMMATRIX projection;
	projection = XMMatrixOrthographicOffCenterLH(0.0f, ScreenSize::ScreenWidth, ScreenSize::ScreenHeight, 0.0f, 0.0f, 1.0f);
	SetProjectionMatrix(projection);
}


void Renderer::SetWorldMatrix(XMMATRIX WorldMatrix)
{
	XMFLOAT4X4 worldf;
	XMStoreFloat4x4(&worldf, XMMatrixTranspose(WorldMatrix));
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &worldf, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(0, 1, &m_WorldBuffer);
}

void Renderer::SetViewMatrix(XMMATRIX ViewMatrix)
{
	XMFLOAT4X4 viewf;
	XMStoreFloat4x4(&viewf, XMMatrixTranspose(ViewMatrix));
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &viewf, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(1, 1, &m_ViewBuffer);
}

void Renderer::SetProjectionMatrix(XMMATRIX ProjectionMatrix)
{
	XMFLOAT4X4 projectionf;
	XMStoreFloat4x4(&projectionf, XMMatrixTranspose(ProjectionMatrix));
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, NULL, &projectionf, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(2, 1, &m_ProjectionBuffer);
}



void Renderer::SetMaterial( MATERIAL Material )
{
	m_DeviceContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &Material, 0, 0 );
}

void Renderer::SetLight( LIGHT Light )
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, NULL, &Light, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(4, 1, &m_LightBuffer);
	m_DeviceContext->PSSetConstantBuffers(4, 1, &m_LightBuffer);
}





void Renderer::CreateVertexShader( ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName )
{

	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	HRESULT hr = m_Device->CreateVertexShader(
		buffer,
		fsize,
		NULL,
		VertexShader
	);

#ifdef _DEBUG
	if (SUCCEEDED(hr) && VertexShader && *VertexShader)
	{
		DX11_SET_NAME(*VertexShader, FileName);
	}
#endif

	if (VertexLayout != nullptr)
	{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(layout,
		numElements,
		buffer,
		fsize,
		VertexLayout);
#ifdef _DEBUG
	if (VertexLayout && *VertexLayout)
	{
		std::string layoutName = std::string(FileName) + " : InputLayout";
		DX11_SET_NAME(*VertexLayout, layoutName.c_str());
	}
#endif
	}
	delete[] buffer;
}


void Renderer::CreateParticleVertexShader(
	ID3D11VertexShader** VertexShader,
	ID3D11InputLayout** InputLayout,
	const char* FileName)
{
	// (ファイル読み込みとCreateVertexShaderの呼び出しは同じ)
	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);

	// インスタンシング専用のレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC instancingLayout[] = {
		// --- スロット0: Per-Vertex Data ---
			// m_QuadVertexBuffer に対応。各頂点ごとに読み込まれる。
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },

			// --- スロット1: Per-Instance Data ---
			// m_ParticleBufferA/B に対応。各パーティクル(インスタンス)ごとに1回だけ読み込まれる。
			{ "INSTANCE_POS",  0, DXGI_FORMAT_R32G32B32_FLOAT,   1, 0,                            D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_VEL",  0, DXGI_FORMAT_R32G32B32_FLOAT,   1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_LIFE", 0, DXGI_FORMAT_R32_FLOAT,         1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_AGE",  0, DXGI_FORMAT_R32_FLOAT,         1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_SIZE", 0, DXGI_FORMAT_R32_FLOAT,		 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_ROT",  0, DXGI_FORMAT_R32_FLOAT,		 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_AVEL",  0, DXGI_FORMAT_R32_FLOAT,		 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_ACTIVE",  0, DXGI_FORMAT_R32_UINT,		 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	m_Device->CreateInputLayout(
		instancingLayout,
		ARRAYSIZE(instancingLayout),
		buffer,
		fsize,
		InputLayout
	);

	delete[] buffer;
}



void Renderer::CreatePixelShader( ID3D11PixelShader** PixelShader, const char* FileName )
{
	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	HRESULT hr = m_Device->CreatePixelShader(
		buffer,
		fsize,
		NULL,
		PixelShader
	);

#ifdef _DEBUG
	if (SUCCEEDED(hr) && PixelShader && *PixelShader)
	{
		DX11_SET_NAME(*PixelShader, FileName);
	}
#endif

	delete[] buffer;
}



void Renderer::CreateComputeShader(ID3D11ComputeShader** computeShader, const char* fileName)
{
	FILE* file;
	long int fsize;

	file = fopen(fileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateComputeShader(buffer, fsize, NULL, computeShader);

	delete[] buffer;
}

void Renderer::SetState_RayDepth()
{//深度保存用

	// 設定した全てのバッファをクリア
	float sceneClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->ClearRenderTargetView(Renderer::GetSceneColorRTV(), sceneClearColor);
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->ClearRenderTargetView(Renderer::m_sceneNormalRTV, ClearColor);
	m_DeviceContext->ClearRenderTargetView(Renderer::m_sceneWorldPosRTV, ClearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// レンダーターゲット: 通常のバックバッファと深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(0, nullptr, m_DepthStencilView);


	// ブレンドステート: ブレンドを無効化
	m_DeviceContext->OMSetBlendState(NULL, nullptr, 0xFFFFFFFF);

	// デプスステンシルステート: 深度テストと深度書き込みの両方を有効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
}

void Renderer::SetState_Opaque()
{//不透明

	ID3D11RenderTargetView* rtvs[] = {
		m_sceneColorRTV,
		m_sceneNormalRTV,
		m_sceneWorldPosRTV
	};
	// レンダーターゲット: 通常のバックバッファと深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(3, rtvs, m_DepthStencilView);


	// ブレンドステート: ブレンドを無効化
	m_DeviceContext->OMSetBlendState(NULL, nullptr, 0xFFFFFFFF);

	// デプスステンシルステート: 深度テストと深度書き込みの両方を有効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);

	// サンプラー
	m_DeviceContext->PSSetSamplers(0, 1, &m_samplerState);

	//ブルームリセット
	BloomColor bc;
	bc.bloomColor = XMFLOAT4(0, 0, 0, 0);
	Renderer::UpdateBloomColor(bc);
}

void Renderer::SetState_Opaque_B()
{//ブルーム有

	ID3D11RenderTargetView* rtvs[] = {
		m_sceneColorRTV,
		m_sceneNormalRTV,
		m_sceneWorldPosRTV,
		 Renderer::GetBloomContributionRTV()
	};
	// レンダーターゲット: 通常のバックバッファと深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(4, rtvs, m_DepthStencilView);


	// ブレンドステート: ブレンドを無効化
	m_DeviceContext->OMSetBlendState(NULL, nullptr, 0xFFFFFFFF);

	// デプスステンシルステート: 深度テストと深度書き込みの両方を有効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);

	// サンプラー
	m_DeviceContext->PSSetSamplers(0, 1, &m_samplerState);
}

void Renderer::SetState_G_Buffer()
{

	// ブレンドステート: ブレンドを無効化
	m_DeviceContext->OMSetBlendState(NULL, nullptr, 0xFFFFFFFF);

	// デプスステンシルステート: 深度テストと深度書き込みの両方を有効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
}

void Renderer::SetState_OitAccum()
{
	// レンダーターゲット: OIT用の2枚のテクスチャをセット
	ID3D11RenderTargetView* rtvs[] = { m_accumRTV, m_revealRTV };
	m_DeviceContext->OMSetRenderTargets(2, rtvs, m_DepthStencilView);

	float clearColorZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearColorOne[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Accumulationは「0.0」で初期化（色が足されていくため）
	m_DeviceContext->ClearRenderTargetView(m_accumRTV, clearColorZero);

	// Revealageは「1.0」で初期化（最初は背景が100%見えているため）
	m_DeviceContext->ClearRenderTargetView(m_revealRTV, clearColorOne);


	// ブレンドステート
	m_DeviceContext->OMSetBlendState(m_oitAccumBlendState, nullptr, 0xFFFFFFFF);


	// デプスステンシルステート: 深度テストON、書き込みOFF
	m_DeviceContext->OMSetDepthStencilState(m_depthStateReadOnly, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
}

void Renderer::SetState_OitComposite()
{
	ID3D11RenderTargetView* rtvs[] = {
	  Renderer::GetSceneLitColorRTV(),
	   Renderer::GetBloomContributionRTV()   // 描画先1: ブルームの強さ
	};
	// レンダーターゲット: 通常のバックバッファに戻す（深度バッファは不要）
	m_DeviceContext->OMSetRenderTargets(2, rtvs, nullptr);


	// ブレンドステート: 通常のアルファブレンドを有効化
	m_DeviceContext->OMSetBlendState(m_BlendState, nullptr, 0xFFFFFFFF);

	// デプスステンシルステート: 深度テストを無効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisabled, 0);
}

void Renderer::SetState_UI()
{
	//
	ID3D11RenderTargetView* rtvs[] = {
	   m_RenderTargetView,
	};
	// レンダーターゲット
	m_DeviceContext->OMSetRenderTargets(1, rtvs, nullptr);

	//UIはカリング無しでも良いってまじ？
	Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsNull());

	// ブレンドステート: 通常のアルファブレンドを有効化
	m_DeviceContext->OMSetBlendState(m_BlendState, nullptr, 0xFFFFFFFF);

	//サンプラー
	Renderer::GetDeviceContext()->PSSetSamplers(0, 1, &m_linearSampler);

	// デプスステンシルステート: 深度テストを無効化
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisabled, 0);
}


void Renderer::SetState_PointLight()
{


	// 描画先: ライティング結果RTV (深度バッファは読み取り専用でセット)
	// (ライトはG-bufferではなく、合成結果の絵に対して描画する)
	m_DeviceContext->OMSetRenderTargets(1, &m_sceneLitColorRTV, m_DepthStencilView);

	// ブレンド: 加算合成
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->OMSetBlendState(m_additiveBlendState, blendFactor, 0xFFFFFFFF);

	// 深度: ライト用 (書き込みなし、GREATER_EQUAL)
	m_DeviceContext->OMSetDepthStencilState(m_depthStateLight, 0);

	// ラスタライザ: 表面カリング (CULL_FRONT)
	// 球体の「裏側」を描画するため、表面を消す m_rsFront を使う
	m_DeviceContext->RSSetState(m_rsFront);



	Renderer::GetDeviceContext()->IASetInputLayout(m_litLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_pointLightVS, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_pointLightPS, NULL, 0);

	// 3. G-bufferリソース設定
	ID3D11ShaderResourceView* srvs[] = {
		Renderer::GetSceneColorSRV(),
		Renderer::GetSceneNormalSRV(),
		Renderer::GetSceneWorldPosSRV()
	};
	m_DeviceContext->PSSetShaderResources(0, 3, srvs);

	// 4. サンプラー
	m_DeviceContext->PSSetSamplers(0, 1, &m_samplerState);
}


void Renderer::SetState_SKY()
{
	ID3D11RenderTargetView* rtv = Renderer::GetSceneLitColorRTV();
	//深度バッファ(DSV)をセットする (比較のため)
	m_DeviceContext->OMSetRenderTargets(1, &rtv, m_DepthStencilView);

	m_DeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);


}

void Renderer::SetState_TransparentEffect()
{
	ID3D11RenderTargetView* rtvs[] = {
	 Renderer::GetSceneLitColorRTV(),
	   Renderer::GetBloomContributionRTV()   // 描画先1: ブルームの強さ
	};
	// レンダーターゲット: 通常のバックバッファと、深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(2, rtvs, m_DepthStencilView);

	// ブレンドステート: 通常のアルファブレンドを有効化
	m_DeviceContext->OMSetBlendState(m_BlendState, nullptr, 0xFFFFFFFF);

	// 
	m_DeviceContext->OMSetDepthStencilState(m_depthStateReadOnly, 0);

}

void Renderer::SetState_Cloud()
{
	ID3D11RenderTargetView* rtvs = m_sceneLitColorRTV;
	
	// レンダーターゲット: 通常のバックバッファと、深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(1, &rtvs, nullptr);

	// ブレンドステート: 通常のアルファブレンドを有効化
	m_DeviceContext->OMSetBlendState(m_BlendState, nullptr, 0xFFFFFFFF);

	// 
	m_DeviceContext->OMSetDepthStencilState(m_depthStateReadOnly, 0);

}

void Renderer::SetState_ParticleAdditive()
{
	ID3D11RenderTargetView* rtvs[] = {
	 Renderer::GetSceneLitColorRTV(),
	   Renderer::GetBloomContributionRTV()
	};
	// レンダーターゲット: 通常のバックバッファと、深度バッファをセット
	m_DeviceContext->OMSetRenderTargets(2, rtvs, m_DepthStencilView);

	// ブレンドステート: 通常のアルファブレンドを有効化
	m_DeviceContext->OMSetBlendState(m_additiveBlendState, nullptr, 0xFFFFFFFF);

	// 
	m_DeviceContext->OMSetDepthStencilState(m_depthStateReadOnly, 0);
}

void Renderer::SetState_ParticlePMA()
{
	ID3D11RenderTargetView* rtvs[] = {
	 Renderer::GetSceneLitColorRTV(),
	   Renderer::GetBloomContributionRTV()   // 描画先1: ブルームの強さ
	};
	//
	m_DeviceContext->OMSetRenderTargets(2, rtvs, m_DepthStencilView);

	// 
	m_DeviceContext->OMSetBlendState(m_blendStatePMA, nullptr, 0xFFFFFFFF);

	// 
	m_DeviceContext->OMSetDepthStencilState(m_depthStateReadOnly, 0);
}

void Renderer::SetState_Raymarching()
{
	ID3D11RenderTargetView* rtvs[] = {
		m_sceneColorRTV,    // Albedo
		m_sceneNormalRTV,   // Normal
		m_sceneWorldPosRTV  // WorldPosition
	};
	m_DeviceContext->OMSetRenderTargets(3, rtvs, nullptr);

	// ブレンドは無効
	m_DeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	// 深度テストは行うが、深度の書き込みは無効化
	//    (ポリゴンの深度と比較するが、フラクタル自身の深度は書き込まない)
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
	//m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisabled, 0);

}



//フルスク切り替え
void Renderer::ToggleFullscreen(HWND hWnd)
{
	m_isFullscreen = !m_isFullscreen;

	if (m_isFullscreen)
	{
		GetWindowRect(hWnd, &m_windowedRect);

		SetWindowLongPtr(
			hWnd,
			GWL_STYLE,
			WS_POPUP | WS_VISIBLE
		);

		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMonitor, &mi);

		SetWindowPos(
			hWnd,
			HWND_TOP,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED
		);
	}
	else
	{
		SetWindowLongPtr(
			hWnd,
			GWL_STYLE,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE
		);

		SetWindowPos(
			hWnd,
			HWND_NOTOPMOST,
			m_windowedRect.left,
			m_windowedRect.top,
			m_windowedRect.right - m_windowedRect.left,
			m_windowedRect.bottom - m_windowedRect.top,
			SWP_FRAMECHANGED
		);
	}

	ShowWindow(hWnd, SW_SHOW); // 念のため
}

void Renderer::UpdateZoneBuffer(const ZONE& data)
{
	m_DeviceContext->UpdateSubresource(m_ZoneBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(5, 1, &m_ZoneBuffer); // スロット5にセット
	m_DeviceContext->CSSetConstantBuffers(5, 1, &m_ZoneBuffer);
}

void Renderer::UpdateBlurParams(const BlurParams& data)
{
	m_DeviceContext->UpdateSubresource(m_blurParamsBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(6, 1, &m_blurParamsBuffer);

}

void Renderer::UpdateCompositeParams(const CompositeParams& data)
{
	m_DeviceContext->UpdateSubresource(m_compositeParamsBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(7, 1, &m_compositeParamsBuffer);
}

void Renderer::UpdateBloomColor(const BloomColor& data)
{
	m_DeviceContext->UpdateSubresource(m_bloomColorBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(8, 1, &m_bloomColorBuffer);
}

void Renderer::UpdateRaymarchingParams(const RaymarchingParams& data)
{
	m_DeviceContext->UpdateSubresource(m_raymarchingParamsBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(9, 1, &m_raymarchingParamsBuffer);
}


void Renderer::UpdatePointLight(const PointLightParams& data)
{
	m_DeviceContext->UpdateSubresource(m_pointLightBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(11, 1, &m_pointLightBuffer);
}

void Renderer::UpdateCloudBuffer(const CloudObjParams& data)
{
	m_DeviceContext->UpdateSubresource(m_cloudBuffer, 0, nullptr, &data, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(12, 1, &m_cloudBuffer);
}



void Renderer::UpdateCsmParams(const CsmParams& data)
{
	CsmParams dataForGPU;
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		// 1. 保存用(XMFLOAT4X4) から 計算用(XMMATRIX) に変換して読み込む
		XMMATRIX mat = XMLoadFloat4x4(&data.matCascadeLightVP[i]);

		// 2. 計算（転置）する
		mat = XMMatrixTranspose(mat);

		// 3. 計算用(XMMATRIX) から 保存用(XMFLOAT4X4) に変換して書き込む
		XMStoreFloat4x4(&dataForGPU.matCascadeLightVP[i], mat);
	}

	// こっちはそのままでOK（XMFLOAT4同士なら代入できる）
	dataForGPU.CascadeSplits = data.CascadeSplits;

	m_DeviceContext->UpdateSubresource(m_csmParamsBuffer, 0, nullptr, &dataForGPU, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(10, 1, &m_csmParamsBuffer);
}


void Renderer::DrawFullscreenQuad()
{
	// 頂点バッファは使わない
	m_DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 3頂点で三角形を1枚描画（これがスクリーン全体を覆う）
	m_DeviceContext->Draw(3, 0);
}


void Renderer::DebugDrawOBB(const Obb& obb, const XMFLOAT4& color)
{
	//一旦削除
}



void Renderer::DrawFullscreenQuad(const XMFLOAT4& color)
{
	

	auto* context = GetDeviceContext();
	auto* device = GetDevice();

	// --- 頂点バッファの作成 (初回のみ) ---
	if (m_FullscreenVB == nullptr)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC; // Map/Unmapで中身を書き換えるためDYNAMICに
		bd.ByteWidth = sizeof(VERTEX_3D) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPUから書き込むため
		device->CreateBuffer(&bd, nullptr, &m_FullscreenVB);
	}

	// --- 頂点データの色を更新 ---
	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(m_FullscreenVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VERTEX_3D* vertices = (VERTEX_3D*)msr.pData;
	float w = (float)ScreenSize::ScreenWidth;
	float h = (float)ScreenSize::ScreenHeight;
	vertices[0] = { {0, 0, 0}, {0,0,-1}, color, {0,0} }; // 左上
	vertices[1] = { {w, 0, 0}, {0,0,-1}, color, {1,0} }; // 右上
	vertices[2] = { {0, h, 0}, {0,0,-1}, color, {0,1} }; // 左下
	vertices[3] = { {w, h, 0}, {0,0,-1}, color, {1,1} }; // 右下
	context->Unmap(m_FullscreenVB, 0);

	// --- 描画ステートの設定 ---
	context->OMSetBlendState(GetBlendState(), nullptr, 0xFFFFFFFF); // 通常のアルファブレンド
	context->OMSetDepthStencilState(GetDepthStateDisabled(), 0);   // 深度テスト無効

	// unlitシェーダー(3Dモデル用)と、それに対応するインプットレイアウトを使用
	context->IASetInputLayout(GetUnlitLayout()); // unlitシェーダー用のレイアウトを取得
	context->VSSetShader(GetUnlitVS(), nullptr, 0);
	context->PSSetShader(GetUnlitPS(), nullptr, 0);

	// --- 行列とマテリアルの設定 ---
	SetWorldViewProjection2D(); // 2D描画用の行列に設定

	MATERIAL material = {};
	material.Diffuse = { 1,1,1,1 };
	material.TextureEnable = false;
	SetMaterial(material);

	// --- 描画 ---
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_FullscreenVB, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->Draw(4, 0);
}



void Renderer::DrawFullscreenQuadRay(const XMFLOAT4& color)
{
	//過去のレイマーチング用

	auto* context = GetDeviceContext();
	auto* device = GetDevice();

	// --- 頂点バッファの作成 (初回のみ) ---
	if (m_rFullscreenVB == nullptr)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC; // Map/Unmapで中身を書き換えるためDYNAMICに
		bd.ByteWidth = sizeof(VERTEX_3D) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPUから書き込むため
		device->CreateBuffer(&bd, nullptr, &m_rFullscreenVB);
	}

	// --- 頂点データの色を更新 ---
	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(m_rFullscreenVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VERTEX_3D* vertices = (VERTEX_3D*)msr.pData;
	float w = (float)ScreenSize::ScreenWidth;
	float h = (float)ScreenSize::ScreenHeight;
	vertices[0] = { {0, 0, 0}, {0,0,-1}, color, {0,0} }; // 左上
	vertices[1] = { {w, 0, 0}, {0,0,-1}, color, {1,0} }; // 右上
	vertices[2] = { {0, h, 0}, {0,0,-1}, color, {0,1} }; // 左下
	vertices[3] = { {w, h, 0}, {0,0,-1}, color, {1,1} }; // 右下
	context->Unmap(m_rFullscreenVB, 0);

	// --- 描画ステートの設定 ---
	context->OMSetBlendState(GetBlendState(), nullptr, 0xFFFFFFFF); // 通常のアルファブレンド
	context->OMSetDepthStencilState(GetDepthStateDisabled(), 0);   // 深度テスト無効

	// unlitシェーダー(3Dモデル用)と、それに対応するインプットレイアウトを使用
	context->IASetInputLayout(GetUnlitLayout()); // unlitシェーダー用のレイアウトを取得
	context->VSSetShader(m_rayTestVS, nullptr, 0);
	context->PSSetShader(m_rayTestPS, nullptr, 0);

	// --- 行列とマテリアルの設定 ---
	SetWorldViewProjection2D(); // 2D描画用の行列に設定

	MATERIAL material = {};
	material.Diffuse = { 1,1,1,1 };
	material.TextureEnable = false;
	SetMaterial(material);

	// --- 描画 ---
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_rFullscreenVB, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->Draw(4, 0);
}



void Renderer::CreateVolumeNoiseTexture()
{
	// 解像度
#ifdef _DEBUG
	int size = 32; // 計算量やばいからデバック時は小さく
#else
	int size = 128;
#endif
	int width = size;
	int height = size;
	int depth = size;

	std::vector<unsigned char> data(width * height * depth);

	// --- ワーリーノイズ生成 ---

	// 1. 空間をいくつかの「セル」に分割し、各セルに1個「種」を撒く
	//    数が少ないほど「大きな雲」になり、多いほど「細かい雲」になる
	int numCells = 8; // 8x8x8 のグリッド
	float cellSize = (float)size / numCells;

	// 各セルの「種」の座標を記憶する配列
	struct Point { float x, y, z; };
	std::vector<Point> seedPoints;

	for (int z = 0; z < numCells; z++) {
		for (int y = 0; y < numCells; y++) {
			for (int x = 0; x < numCells; x++) {
				// セルの中のランダムな位置に点を置く
				float px = (x + (rand() % 100 / 100.0f)) * cellSize;
				float py = (y + (rand() % 100 / 100.0f)) * cellSize;
				float pz = (z + (rand() % 100 / 100.0f)) * cellSize;
				seedPoints.push_back({ px, py, pz });
			}
		}
	}

	// 2. 全ピクセルについて「最も近い種までの距離」を計算する
	//    (本来は隣接セルだけ見れば高速ですが、実装簡略化のため全探索します。起動時のみなので許容)
	for (int z = 0; z < depth; z++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float minDist = 99999.0f;

				// 最も近い種を探す (全探索)
				// ※高速化するなら「自分のセルと隣のセル」だけ見るループに変える
				for (const auto& p : seedPoints)
				{
					// --- 修正後：シームレス対応 ---
					float dx = std::abs(x - p.x);
					float dy = std::abs(y - p.y);
					float dz = std::abs(z - p.z);

					// サイズの半分より遠い場合は、逆側から回ったほうが近い（ループ処理）
					if (dx > size * 0.5f) dx = size - dx;
					if (dy > size * 0.5f) dy = size - dy;
					if (depth > 0 && dz > size * 0.5f) dz = size - dz; // depthが1より大きい場合のみ

					float distSq = dx * dx + dy * dy + dz * dz;
					if (distSq < minDist) minDist = distSq;
				}

				// 距離を 0.0～1.0 に変換
				// 雲は「中心が濃い」ので、距離が近いほど値を大きくする (1.0 - dist)
				float dist = sqrt(minDist);
				float maxDist = cellSize; // セルサイズくらいで正規化
				float value = 1.0f - (dist / maxDist);
				if (value < 0) value = 0;

				// 0～255 に変換して格納
				int index = x + (y * width) + (z * width * height);
				data[index] = (unsigned char)(value * 255.0f);
			}
		}
	}

	// --- Texture3D 作成 (変更なし) ---
	D3D11_TEXTURE3D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data.data();
	initData.SysMemPitch = width * sizeof(unsigned char);
	initData.SysMemSlicePitch = width * height * sizeof(unsigned char);

	HRESULT hr = m_Device->CreateTexture3D(&desc, &initData, &m_noiseTexture);
	if (FAILED(hr)) return;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;

	m_Device->CreateShaderResourceView(m_noiseTexture, &srvDesc, &m_noiseSRV);
}

void Renderer::DrawVolumetricCloud(XMMATRIX v, XMMATRIX p,Vector3 pos)
{
	ID3D11DeviceContext* context = GetDeviceContext();

	// OIT用ステート設定
	ID3D11RenderTargetView* rtvs[] = { m_accumRTV, m_revealRTV };
	context->OMSetRenderTargets(2, rtvs, m_DepthStencilView); // 深度はReadOnly推奨
	context->OMSetBlendState(m_oitAccumBlendState, nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_depthStateReadOnly, 0);


	//  定数バッファの準備
	{
		CloudParams cb;

		// 現在のView行列とProjection行列を取得して掛け合わせる
		XMMATRIX view = v;
		XMMATRIX proj = p;
		XMMATRIX viewProj = view * proj;

		// 逆行列を計算
		XMVECTOR det;
		// 計算結果は一旦ローカルの XMMATRIX 変数で
		XMMATRIX invMat = XMMatrixInverse(&det, viewProj);

		// HLSL用に転置
		invMat = XMMatrixTranspose(invMat);

		// ここ修正計算が終わった XMMATRIX を、構造体の XMFLOAT4X4 に格納する
		XMStoreFloat4x4(&cb.InvViewProj, invMat);

		cb.CloudColor = XMFLOAT4(0.9f, 0.95f, 1.0f, 1.0f);
		cb.EyePos = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);

		if (!cloudCB) {
			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = sizeof(CloudParams);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			GetDevice()->CreateBuffer(&desc, nullptr, &cloudCB);
		}
		context->UpdateSubresource(cloudCB, 0, nullptr, &cb, 0, 0);
		context->PSSetConstantBuffers(13, 1, &cloudCB); 
	}

	// シェーダー設定
	context->VSSetShader(GetPostEffectVS(), nullptr, 0);
	context->PSSetShader(m_cloudPS, nullptr, 0);

	// リソース設定
	ID3D11ShaderResourceView* srvs[] = {
		m_noiseSRV,              // t0: 3Dノイズ
		GetSceneWorldPosSRV()    // t1: G-Buffer
	};
	context->PSSetShaderResources(0, 2, srvs);

	// 描画
	DrawFullscreenQuad();

	// 後片付け
	ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr };
	context->PSSetShaderResources(0, 2, nullSRVs);
}

void Renderer::InitPointLightSystem()
{
	// 共通モデルの読み込み
	// 各PointLightでLoadするのではなく、ここで1回だけ読み込む
	m_commonSphereModel = new ModelRenderer();
	m_commonSphereModel->Load("asset\\pointLight.obj");
	// StructuredBufferを作成
	// D3D11_BIND_SHADER_RESOURCE と D3D11_RESOURCE_MISC_BUFFER_STRUCTURED が必要
	// Dynamic Usageで作成し、毎フレーム Map/Unmap で書き換えるのが簡単

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(PointLightParams) * 1024; // MAX_LIGHTS
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(PointLightParams);

	m_Device->CreateBuffer(&desc, nullptr, &m_lightInstBuffer);

	// SRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements = 1024;

	m_Device->CreateShaderResourceView(m_lightInstBuffer, &srvDesc, &m_lightInstSRV);

}
void Renderer::AddPointLight(const PointLightParams& data)
{
	m_lightList.push_back(data);
}
void Renderer::DrawPointLightsInstanced()
{
	// ライトが1つもないなら描画しない
	if (m_lightList.empty())
	{
		return;
	}

	// 最大数チェック
	if (m_lightList.size() > 1024) m_lightList.resize(1024);

	// ---------------------------------------------------------
	// GPUバッファの更新 (Map / Unmap)
	// ---------------------------------------------------------
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = m_DeviceContext->Map(m_lightInstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		// std::vector の中身をそのままGPUバッファにコピー
		// (構造体のメモリレイアウトが一致している前提)
		memcpy(mapped.pData, m_lightList.data(), sizeof(PointLightParams) * m_lightList.size());

		m_DeviceContext->Unmap(m_lightInstBuffer, 0);
	}


	// ---------------------------------------------------------
	// 描画ステートの設定
	// ---------------------------------------------------------
	// 既存のポイントライト用ステート設定
	SetState_PointLight();


	// ---------------------------------------------------------
	// リソースのバインド
	// ---------------------------------------------------------

	// VSの t11 スロットにライト情報リストをセット
	m_DeviceContext->VSSetShaderResources(11, 1, &m_lightInstSRV);

	// 球体モデルの頂点・インデックスバッファをセット
	if (m_commonSphereModel)
	{
		m_commonSphereModel->BindBuffers(m_DeviceContext);

		// ---------------------------------------------------------
		// インスタンシング描画実行
		// ---------------------------------------------------------
		unsigned int indexCount = m_commonSphereModel->GetIndexCount();
		unsigned int instanceCount = (unsigned int)m_lightList.size();

		// 引数: (インデックス数, インスタンス数(ライトの数), 開始インデックス, ベース頂点, 開始インスタンス)
		m_DeviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
	}


	// ---------------------------------------------------------
	// 後始末
	// ---------------------------------------------------------

	// 次のフレームのためにリストをクリア
	m_lightList.clear();

	// SRVを解除しておく
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_DeviceContext->VSSetShaderResources(11, 1, &nullSRV);
}

