#pragma once


#include "obb.h"


#define SAFE_RELEASE(p) \
    if ((p) != nullptr) { \
        (p)->Release();   \
        (p) = nullptr;   \
    }

struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};


struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	BOOL		TextureEnable;
	float		Dummy[2];
};



struct LIGHT
{
	BOOL		Enable;
	BOOL        CastsShadows; //(影を生成するかどうか)
	BOOL		Dummy[2];
	XMFLOAT4	Direction;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;
};

struct ZONE
{
	XMFLOAT4 CameraPosition;
	float TotalTime;
	float DeltaTime;
	BOOL		Dummy[2];
	XMFLOAT4 PlayerPosition;
	XMFLOAT4 PlayerVelocity;
};


// GPUに送るパーティクルのデータ構造
struct Particle
{
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	float Life;
	float Age; // 生成されてからの時間
	float Size;
	float Rotaion;
	float AngularVelocity;
	UINT  IsActive;
};

struct BlurParams
{
	XMFLOAT2 blurDirection;
	XMFLOAT2 texelSize;
};


struct CompositeParams
{
	float intensityA;
	float intensityB;
	BOOL		Dummy[2];
};


struct BloomColor
{
	XMFLOAT4 bloomColor;
};

struct RaymarchingParams
{
	XMFLOAT3 cameraPosition;
	float time;
	
	XMFLOAT3 cameraForward;
	float    dummy1;

	XMFLOAT3 cameraRight;
	float    dummy2;

	XMFLOAT3 cameraUp;
	float    dummy3;

	XMFLOAT4X4 r_inverseViewProjection;
	XMFLOAT4X4 r_inverseProjection;
};


struct CsmParams
{
	XMFLOAT4X4 matCascadeLightVP[3];//CASCADE_COUNT
	XMFLOAT4 CascadeSplits;
};

// --- ポイントライト専用パラメータ (b11) ---
struct PointLightParams
{
	XMFLOAT3 LightPos;       // ライトの中心座標
	float  LightRange;     // ライトの半径
	XMFLOAT3 LightColor;     // ライトの色 (RGB)
	float  LightIntensity; // 光の強さ
};

struct CloudParams
{
	XMFLOAT4X4 InvViewProj; // UVからワールド座標を復元するための逆行列
	XMFLOAT4 CloudColor;  // 雲の色
	XMFLOAT4 EyePos;      // カメラ位置
};

struct CloudObjParams 
{
	XMFLOAT4 Color;
	XMFLOAT4 Center;
	XMFLOAT4 Size;
	float    Density;
	XMFLOAT3 NoiseOffset;
};

class Renderer
{
public:
	static constexpr int bloomLevel = 8;
	static constexpr int CASCADE_COUNT = 3;//これ変更する場合にはcommonのCsmParamsの配列番号も変更する
	static constexpr UINT SHADOW_MAP_SIZE = 2048; //2048x2048

private:
	


	static D3D_FEATURE_LEVEL       m_FeatureLevel;

	static ID3D11Device*           m_Device;
	static ID3D11DeviceContext*    m_DeviceContext;
	static IDXGISwapChain*         m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	//定数バッファ
	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;
	static ID3D11Buffer*			m_ZoneBuffer;
	static ID3D11Buffer*			m_blurParamsBuffer;
	static ID3D11Buffer*			m_compositeParamsBuffer;
	static ID3D11Buffer*			m_bloomColorBuffer;
	static ID3D11Buffer*			m_raymarchingParamsBuffer;
	static ID3D11Buffer*			m_csmParamsBuffer;
	static ID3D11Buffer*			m_pointLightBuffer;
	static ID3D11Buffer*			m_cloudBuffer;


	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	static ID3D11BlendState*		m_BlendState;
	static ID3D11BlendState*		m_BlendStateATC;

	static ID3D11BlendState* m_additiveBlendState;
	static ID3D11BlendState* m_blendStatePMA;

	static ID3D11BlendState* m_oitAccumBlendState;


	// 半透明描画用
	static ID3D11Texture2D* m_accumTexture;
	static ID3D11Texture2D* m_revealTexture;
	static ID3D11RenderTargetView* m_accumRTV;
	static ID3D11RenderTargetView* m_revealRTV;
	static ID3D11ShaderResourceView* m_accumSRV;
	static ID3D11ShaderResourceView* m_revealSRV;


	static ID3D11DepthStencilState* m_depthStateReadOnly;
	static ID3D11DepthStencilState* m_DepthStateDisabled;

	static ID3D11VertexShader* m_CompositeVS;
	static ID3D11PixelShader* m_CompositePS;
	static ID3D11InputLayout* m_VertexLayout;

	static ID3D11RasterizerState* m_rsBack;
	static ID3D11RasterizerState* m_rsFront;
	static ID3D11RasterizerState* m_rsNone;
	static ID3D11SamplerState* m_samplerState;


	static ID3D11SamplerState* m_linearSampler;


	//
	static bool m_isFullscreen;

	//ブルーム関連
	static ID3D11Texture2D* m_bloomContributionTexture;
	static ID3D11RenderTargetView* m_bloomContributionRTV;
	static ID3D11ShaderResourceView* m_bloomContributionSRV;

	// ダウンサンプル用レンダーターゲット (半分の解像度)
	static ID3D11Texture2D* m_downSampleTexture_Half;
	static ID3D11RenderTargetView* m_downSampleRTV_Half;
	static ID3D11ShaderResourceView* m_downSampleSRV_Half;

	// ダウンサンプル用レンダーターゲット (4分の1の解像度)
	static ID3D11Texture2D* m_downSampleTexture_Quarter;
	static ID3D11RenderTargetView* m_downSampleRTV_Quarter;
	static ID3D11ShaderResourceView* m_downSampleSRV_Quarter;

	static ID3D11VertexShader* m_postEffectVS;
	static ID3D11PixelShader* m_downsamplePS;
	static ID3D11PixelShader* m_blurPS;
	static ID3D11PixelShader* m_bloomCompositePS;

	static ID3D11Texture2D* m_tempTexture_Quarter;
	static ID3D11RenderTargetView* m_tempRTV_Quarter;
	static ID3D11ShaderResourceView* m_tempSRV_Quarter;

	static ID3D11Texture2D* m_sceneColorTexture;
	static ID3D11RenderTargetView* m_sceneColorRTV;
	static ID3D11ShaderResourceView* m_sceneColorSRV;


	static ID3D11PixelShader* m_bloomUpsamplePS;


	static ID3D11PixelShader* m_copyPS;

	//汎用シェーダー
	static ID3D11VertexShader* m_unlitVS;
	static ID3D11PixelShader* m_unlitPS;
	static ID3D11InputLayout* m_unlitLayout; // unlitVS用のレイアウト
	static ID3D11RasterizerState* m_rsWireframe;

	//レイマーチングのテスト
	static ID3D11VertexShader* m_rayTestVS;
	static ID3D11PixelShader * m_rayTestPS;

	//ray
	static ID3D11ShaderResourceView* m_DepthSRV;

	//多段階ブルーム用
	static ID3D11Texture2D* m_downSampleTextures[bloomLevel];
	static ID3D11RenderTargetView* m_downSampleRTVs[bloomLevel];
	static ID3D11ShaderResourceView* m_downSampleSRVs[bloomLevel];

	static ID3D11Texture2D* m_tempTextures[bloomLevel];
	static ID3D11RenderTargetView* m_tempRTVs[bloomLevel];
	static ID3D11ShaderResourceView* m_tempSRVs[bloomLevel];

	

	//g-buffer用
	static ID3D11Texture2D*				m_sceneNormalTexture;
	static ID3D11RenderTargetView*		m_sceneNormalRTV;
	static ID3D11ShaderResourceView*	m_sceneNormalSRV;

	static ID3D11Texture2D*				m_sceneWorldPosTexture;
	static ID3D11RenderTargetView*		m_sceneWorldPosRTV;
	static ID3D11ShaderResourceView*	m_sceneWorldPosSRV;

	static ID3D11Texture2D*				m_sceneLitColorTexture;
	static ID3D11RenderTargetView*		m_sceneLitColorRTV;
	static ID3D11ShaderResourceView*	m_sceneLitColorSRV;


	//shadow
	static ID3D11Texture2D* m_shadowMapTexture;
	static ID3D11DepthStencilView* m_shadowMapDSV[CASCADE_COUNT];
	static ID3D11ShaderResourceView* m_shadowMapSRV;
	static ID3D11RasterizerState* m_shadowRS;
	static ID3D11SamplerState* m_shadowSampler;

	static ID3D11PixelShader* m_gLightingPS;
	static ID3D11PixelShader* m_rayDepthPS;

	static ID3D11PixelShader* m_raymShadowPS;
	static ID3D11VertexShader* m_shadowVS;

	//ライト
	static ID3D11DepthStencilState* m_depthStateLight; // ライトボリューム用（Z判定用）

	static ID3D11PixelShader* m_pointLightPS;
	static ID3D11VertexShader* m_pointLightVS;
	static ID3D11InputLayout* m_litLayout;

	// 3Dテクスチャと、シェーダーリソースビュー(SRV)
	static ID3D11Texture3D* m_noiseTexture;
	static ID3D11ShaderResourceView* m_noiseSRV;

	static ID3D11PixelShader* m_cloudPS; // シェーダー変数

	//フルスク
	static RECT m_windowedRect; // 元のウィンドウサイズ保存用


	static ID3D11SamplerState* m_bloomSampler;
	// テクスチャを作る関数
	static void CreateVolumeNoiseTexture();


	//
	static ID3D11Buffer* m_FullscreenVB;
	static ID3D11Buffer* m_rFullscreenVB;

	//エミッション用のダミーテクスチャ
	static ID3D11ShaderResourceView* m_DummyTextureSRV;

	static ID3D11Buffer* cloudCB;

	// インスタンシング用リソース
	static ID3D11Buffer* m_lightInstBuffer;
	static ID3D11ShaderResourceView* m_lightInstSRV;
	static std::vector<PointLightParams> m_lightList;

	// ライト描画用の共通球体モデル
	static class ModelRenderer* m_commonSphereModel;

public:


	//ループ関連
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	//関数群
	static void SetDepthEnable(bool Enable);
	static void SetATCEnable(bool Enable);
	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(XMMATRIX WorldMatrix);
	static void SetViewMatrix(XMMATRIX ViewMatrix);
	static void SetProjectionMatrix(XMMATRIX ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetLight(LIGHT Light);

	static ID3D11Device* GetDevice( void ){ return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext( void ){ return m_DeviceContext; }

	//シェーダー作成
	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreateParticleVertexShader(ID3D11VertexShader** VertexShader,ID3D11InputLayout** InputLayout,const char* FileName);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);
	static void CreateComputeShader(ID3D11ComputeShader** computeShader, const char* fileName);

	//ステート
	static void SetState_Opaque();
	static void SetState_OitAccum();
	static void SetState_OitComposite();
	static void SetState_UI();
	static void SetState_TransparentEffect();
	static void SetState_ParticleAdditive();
	static void SetState_ParticlePMA();
	static void SetState_G_Buffer();
	static void SetState_RayDepth();
	static void SetState_SKY();
	static void SetState_PointLight();
	static void SetState_Cloud();
	static void SetState_Opaque_B();

	//
	static ID3D11ShaderResourceView* GetAccumSRV() { return m_accumSRV; }
	static ID3D11ShaderResourceView* GetRevealSRV() { return m_revealSRV; }
	static ID3D11VertexShader* GetCompositeVS() { return  m_CompositeVS; }
	static ID3D11PixelShader* GetCompositePS() { return  m_CompositePS; }
	static ID3D11RasterizerState* GetRsBack() { return  m_rsBack; }
	static ID3D11RasterizerState* GetRsFront() { return  m_rsFront; }
	static ID3D11RasterizerState* GetRsNull() { return  m_rsNone; }
	static ID3D11BlendState* GetAdditiveBlendState() { return   m_additiveBlendState; }
	static ID3D11DepthStencilState* GetDepthStateReadOnly() { return  m_depthStateReadOnly; }

	static ID3D11RenderTargetView* GetBloomContributionRTV() { return  m_bloomContributionRTV;}
	static ID3D11RenderTargetView* GetRenderTargetView() { return m_RenderTargetView; }
	static ID3D11DepthStencilView* GetDepthStencilView() { return m_DepthStencilView; }

	// ブルーム用SRV (最終合成で使う)
	static ID3D11ShaderResourceView* GetBloomContributionSRV() { return m_bloomContributionSRV; }

	// プリマルチプライドアルファ用のブレンドステート
	static ID3D11BlendState* GetBlendStatePMA() { return m_blendStatePMA; }

	//フルスク
	static void ToggleFullscreen(HWND hWnd);

	static void DrawCube();

	//各種定数バッファ更新
	static void UpdateZoneBuffer(const ZONE& data);
	static void UpdateBlurParams(const BlurParams& data);
	static void UpdateCompositeParams(const CompositeParams& data);
	static void UpdateBloomColor(const BloomColor& data);
	static void UpdateRaymarchingParams(const RaymarchingParams& data);
	static void UpdateCsmParams(const CsmParams& data);
	static void UpdatePointLight(const PointLightParams& data);
	static void UpdateCloudBuffer(const CloudObjParams& data);

	//一旦ここに
	static float RandomFloat(float min, float max)
	{
		// rand() / RAND_MAX で 0.0～1.0 の値を取得
		return min + (rand() / (float)RAND_MAX) * (max - min);
	}

	static void DrawFullscreenQuad();

	// ダウンサンプル用レンダーターゲット (半分サイズ)
	static ID3D11RenderTargetView* GetDownSampleRTV_Half() { return m_downSampleRTV_Half; }
	static ID3D11ShaderResourceView* GetDownSampleSRV_Half() { return m_downSampleSRV_Half; }

	// ダウンサンプル用レンダーターゲット (4分の1サイズ)
	static ID3D11RenderTargetView* GetDownSampleRTV_Quarter() { return m_downSampleRTV_Quarter; }
	static ID3D11ShaderResourceView* GetDownSampleSRV_Quarter() { return m_downSampleSRV_Quarter; }

	// ポストエフェクト用のシェーダー
	static ID3D11VertexShader* GetPostEffectVS() { return m_postEffectVS; }
	static ID3D11PixelShader* GetDownsamplePS() { return m_downsamplePS; }
	static ID3D11PixelShader* GetBlurPS() { return m_blurPS; }
	static ID3D11PixelShader* GetBloomCompositePS() { return m_bloomCompositePS; }
	static ID3D11PixelShader* GetCopyPS() { return m_copyPS; }

	static ID3D11RenderTargetView* GetTempRTV_Quarter() { return m_tempRTV_Quarter; }
	static ID3D11ShaderResourceView* GetTempSRV_Quarter() { return m_tempSRV_Quarter; }

	static ID3D11RenderTargetView* GetSceneColorRTV() { return m_sceneColorRTV; }
	static ID3D11ShaderResourceView* GetSceneColorSRV() { return m_sceneColorSRV; }


	static void DebugDrawOBB(const Obb& obb, const XMFLOAT4& color);

	static ID3D11VertexShader* GetUnlitVS() { return m_unlitVS; }
	static ID3D11PixelShader* GetUnlitPS() { return m_unlitPS; }
	static ID3D11PixelShader* GetRayTestPS() { return m_rayTestPS; }
	// ウィンドウのバックバッファRTV (最終出力先)
	static ID3D11RenderTargetView* GetBackBufferRTV() { return m_RenderTargetView; }

	// 汎用unlitシェーダー用のインプットレイアウト
	static ID3D11InputLayout* GetUnlitLayout() { return m_unlitLayout; }
	// 通常のアルファブレンド用のブレンドステートを取得
	static ID3D11BlendState* GetBlendState() { return m_BlendState; }

	// 深度テストを完全に無効にするデプスステートを取得
	static ID3D11DepthStencilState* GetDepthStateDisabled() { return m_DepthStateDisabled; }

	static void DrawFullscreenQuad(const XMFLOAT4& color);
	static void DrawFullscreenQuadRay(const XMFLOAT4& color);

	// ダウンサンプル用レンダーターゲット
	static ID3D11RenderTargetView* GetDownSampleRTVs(int num) { return m_downSampleRTVs[num]; }
	static ID3D11ShaderResourceView* GetDownSampleSRVs(int num) { return m_downSampleSRVs[num]; }

	static ID3D11RenderTargetView* GetTempRTVs(int num) { return m_tempRTVs[num]; }
	static ID3D11ShaderResourceView* GetTempSRVs(int num) { return m_tempSRVs[num]; }

	//
	static ID3D11ShaderResourceView* GetDepthSRV() { return m_DepthSRV; }
	static void SetState_Raymarching();

	// --- G-Buffer用 ---
  // Normal (法線)
	static ID3D11RenderTargetView* GetSceneNormalRTV() { return m_sceneNormalRTV; }
	static ID3D11ShaderResourceView* GetSceneNormalSRV() { return m_sceneNormalSRV; }

	// World Position (ワールド座標)
	static ID3D11RenderTargetView* GetSceneWorldPosRTV() { return m_sceneWorldPosRTV; }
	static ID3D11ShaderResourceView* GetSceneWorldPosSRV() { return m_sceneWorldPosSRV; }

	// Lit Color (ライティング結果)
	static ID3D11RenderTargetView* GetSceneLitColorRTV() { return m_sceneLitColorRTV; }
	static ID3D11ShaderResourceView* GetSceneLitColorSRV() { return m_sceneLitColorSRV; }

	// --- シャドウマップ用 ---
	static ID3D11DepthStencilView* GetShadowMapDSV(int num) { return m_shadowMapDSV[num]; }
	static ID3D11ShaderResourceView* GetShadowMapSRV() { return m_shadowMapSRV; }
	static ID3D11RasterizerState* GetShadowRS() { return m_shadowRS; }
	static ID3D11SamplerState* GetShadowSMP() { return m_shadowSampler; }


	static ID3D11PixelShader* GetLightingPS() { return m_gLightingPS; }
	static ID3D11PixelShader* GetRayDepthPS() { return m_rayDepthPS; }
	static ID3D11PixelShader* GetRayShadowPS() { return m_raymShadowPS; }

	static ID3D11VertexShader* GetShadowVS() { return m_shadowVS; }

	static ID3D11DepthStencilState* GetDepthStateEnable() { return m_DepthStateEnable; }

	//
	static ID3D11ShaderResourceView* GetVolumeNoiseSRV() { return m_noiseSRV; }


	static ID3D11SamplerState* GetBloomSampler() { return m_bloomSampler; }
	static ID3D11SamplerState* GetBaseSampler() { return m_samplerState; }


	static ID3D11PixelShader* GetBloomUpsamplePS() { return m_bloomUpsamplePS; }

	static ID3D11ShaderResourceView* GetDummyTextureSRV() { return m_DummyTextureSRV; }

	static ID3D11SamplerState* GetLinearSampler() { return m_linearSampler; }

	// ボリューメトリッククラウド描画
	static void DrawVolumetricCloud(XMMATRIX v, XMMATRIX p, Vector3 pos);

	// 初期化時にバッファ作成とモデル読み込みを行う
	static void InitPointLightSystem();

	// ライトを追加するだけ
	static void AddPointLight(const PointLightParams& data);

	// 一括描画
	static void DrawPointLightsInstanced();
	static void CheckD3D11Leaks();
};
