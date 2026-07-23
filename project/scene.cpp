#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "camera.h"
#include "shadowcamera.h"
#include "player.h"
#include "time.h"
#include "input.h"
#include "imgui.h"
#include "imGuiManager.h"
#include <algorithm>

//レンダリングの描画パスとかもここに
XMMATRIX CalculateCascadeMatrix(Camera* camera, LIGHT light, const float* cascadeSplits, int cascadeInd,
    XMMATRIX* outMatLightProj, // 計算結果の「Proj行列」を格納する場所
    XMVECTOR* outLightPos,     // 計算結果の「ライトの位置」を格納する場所
    XMVECTOR* outLightForward, // 計算結果の「ライトの向き」を格納する場所
    XMVECTOR* outLightRight,
    XMVECTOR* outLightUp,
    float* outLightFarZ
);

void Scene::Init()
{

}


void Scene::Uninit()
{
	for (int i = 0; i < LAYER_MAX; i++)
	{
		for (auto gameObject : m_gameObject[i])
		{
			gameObject->Uninit();
			delete gameObject;
		}
		m_gameObject[i].clear();
	}
	
}

void Scene::Update()
{
    if (Input::GetKeyTrigger('I'))
    {
		drawImgui = !drawImgui;
    }
 
    Camera* camera = GetGameObject<Camera>();
    Player* player = GetGameObject<Player>();
    if (camera != nullptr && player != nullptr)
    {


        ZONE scb;
        scb.CameraPosition = XMFLOAT4(camera->GetPos().x, camera->GetPos().y, camera->GetPos().z, 0.0f);
        scb.TotalTime = Time::TotalTime();
        scb.DeltaTime = Time::GamePlayTime();
        scb.Dummy[0] = Time::GamePlayTime();
        scb.Dummy[1] = Time::GamePlayTime();
      

        Vector3 pPos= player->GetPos();
        scb.PlayerPosition = { pPos.x,pPos.y, pPos.z, 0 };
        Vector3 pVel = player->GetVel();
        scb.PlayerVelocity = { pVel.x,pVel.y, pVel.z, 0 };

        Renderer::UpdateZoneBuffer(scb);
    }

	for (int i = 0; i < LAYER_MAX; i++)
	{
		for (auto gameObject : m_gameObject[i])
		{
			gameObject->Update();
		}
	}
    

	for (int i = 0; i < LAYER_MAX; i++)
	{
		m_gameObject[i].remove_if([](GameObject* object)
			{
				return object->Destroy();
			});
	}
    
}


void Scene::Draw()
{
   
    auto* context = Renderer::GetDeviceContext();
    //カメラ取得
    Camera* camera = GetGameObject<Camera>();
    ShadowCamera* sCamera = GetGameObject<ShadowCamera>();//シャドウカメラ

    if (camera != nullptr)
    {
        // Camera::Draw()から行列設定を分離した、新しい関数
        camera->SetMatrix();

    }
    if (sCamera != nullptr)
    {
        sCamera->SetMatrix();
        sCamera->Update();

    }


    // =============================
    // 0. シャドウマップ生成パス
    // =============================

    float cascadeSplits[Renderer::CASCADE_COUNT + 1]; // 分割点は4つ (Near, split1, split2, Far)

    float cameraNear = camera->GetNearClip(); // 0.1f
    //float cameraFar = camera->GetFarClip();   // 1000.0f
    float cameraFar = 500.0f;   // 1000.0f

    float lambda = 0.9f; // 0.0=リニア, 1.0=対数
    float range = cameraFar - cameraNear;

    for (int i = 0; i <= Renderer::CASCADE_COUNT; ++i)
    {
        float ratio = (float)i / (float)Renderer::CASCADE_COUNT;
        float splitLinear = cameraNear + range * ratio;
        float splitLog = cameraNear * pow(cameraFar / cameraNear, ratio);

        // 2つをブレンドする
        //cascadeSplits[i] = lerp(splitLinear, splitLog, lambda);
        cascadeSplits[i] = splitLinear * (1.0f - lambda) + splitLog * lambda;
    }
    //
    CsmParams csmData;
    csmData.CascadeSplits = XMFLOAT4(cascadeSplits[1], cascadeSplits[2], cascadeSplits[3], 0.0f);


    //1. シャドウマップ用のステートを設定
    context->RSSetState(Renderer::GetShadowRS());

    // 2. シャドウマップ用のビューポートを設定
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)Renderer::SHADOW_MAP_SIZE;
    vp.Height = (float)Renderer::SHADOW_MAP_SIZE;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    XMMATRIX localProjMatrices[Renderer::CASCADE_COUNT];
    XMVECTOR localLightPositions[Renderer::CASCADE_COUNT];
    XMVECTOR localLightForwards[Renderer::CASCADE_COUNT];
    XMVECTOR localLightRights[Renderer::CASCADE_COUNT];
    XMVECTOR localLightUps[Renderer::CASCADE_COUNT];
    float localLightFarZs[Renderer::CASCADE_COUNT];


    //3. CSMのカスケード数だけループ
    for (int i = 0; i < Renderer::CASCADE_COUNT; ++i)
    {
        // 3a. このカスケード(i)用のライト行列を計算する
        XMMATRIX matLightVP = CalculateCascadeMatrix(camera, sCamera->GetLight(), cascadeSplits, i,
            &localProjMatrices[i],
            &localLightPositions[i],
            &localLightForwards[i],
            &localLightRights[i],
            &localLightUps[i],
            &localLightFarZs[i]
            );
       
        //csmData.matCascadeLightVP[i] = matLightVP;
     
        XMStoreFloat4x4(&csmData.matCascadeLightVP[i], matLightVP);

        // 3b. このカスケード用のDSVをセット (RTVはnullptr)
        ID3D11DepthStencilView* dsv = Renderer::GetShadowMapDSV(i);
        context->OMSetRenderTargets(0, nullptr, dsv);
        context->OMSetDepthStencilState(Renderer::GetDepthStateEnable(), 0);

        // 3c. DSVをクリア
        context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

        //View/Projection バッファを「ライト行列」にセット
        Renderer::SetViewMatrix(XMMatrixIdentity()); // (仮: DrawShadowの実装に合わせる)
        Renderer::SetProjectionMatrix(matLightVP);   // (仮: DrawShadowの実装に合わせる)


		//　3dオブジェクトのシャドウマップ用描画関数
        for (auto gameObject : m_gameObject[l_NOT_TOUMEI]) {
            gameObject->DrawShadow();
        }

		for (auto gameObject : m_gameObject[l_NOT_TOUMEI_BLOOM]) {
			gameObject->DrawShadow();
		}
       

    }

    Renderer::UpdateCsmParams(csmData);

    // 1. ビューポートをスクリーンサイズに戻す
    vp.Width = (float)ScreenSize::ScreenWidth;
    vp.Height = (float)ScreenSize::ScreenHeight;
    context->RSSetViewports(1, &vp);

    // 2. ラスタライザステートを通常の裏面カリングに戻す
    context->RSSetState(Renderer::GetRsBack());


   //ブルームテクスチャクリア
    float bloomClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Renderer::GetDeviceContext()->ClearRenderTargetView(Renderer::GetBloomContributionRTV(), bloomClearColor);


    // =============================
    // レイマーチングの深度保存用パス（現在利用しておらず）
    // =============================

    Renderer::SetState_RayDepth();

    //レイマーチング用のシェーダーをセット
    context->VSSetShader(Renderer::GetPostEffectVS(), nullptr, 0);
    context->PSSetShader(Renderer::GetRayDepthPS(), nullptr, 0);


    // レイマーチングに必要な情報を定数バッファで渡す
    if (camera != nullptr)
    {
        camera->SetMatrix();

        RaymarchingParams cb;

        XMMATRIX view = camera->GetViewMatrix();
        XMMATRIX proj = camera->GetProjectionMatrix();

        // ビュー逆行列を計算して転置
        XMMATRIX invView = XMMatrixInverse(nullptr, view);
        invView = XMMatrixTranspose(invView);
        XMStoreFloat4x4(&cb.r_inverseViewProjection, invView);

        // プロジェクション逆行列も計算して転置
        XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
        invProj = XMMatrixTranspose(invProj);
        XMStoreFloat4x4(&cb.r_inverseProjection, invProj);

        cb.cameraPosition.x = camera->GetPos().x;
        cb.cameraPosition.y = camera->GetPos().y;
        cb.cameraPosition.z = camera->GetPos().z;
        cb.time = Time::TotalTime();
        cb.cameraForward = XMFLOAT3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z);
        cb.cameraRight = XMFLOAT3(camera->GetRight().x, camera->GetRight().y, camera->GetRight().z);
        cb.cameraUp = XMFLOAT3(camera->GetUp().x, camera->GetUp().y, camera->GetUp().z);
        cb.dummy1 = ScreenSize::ScreenWidth;
        cb.dummy2 = ScreenSize::ScreenHeight;
        cb.dummy3 = Time::GamePlayTime();

        Renderer::UpdateRaymarchingParams(cb);
    }



    //レイマーチング深度描画
    //Renderer::DrawFullscreenQuad();

    // =============================
    // 不透明パス
    // =============================
    
    // 不透明オブジェクト用の描画設定を呼び出す
    Renderer::SetState_Opaque();

    // 不透明レイヤーのオブジェクトを描画
    for (auto gameObject : m_gameObject[l_FIRST]) {//カメラなど最初に描画するもの
        gameObject->Draw();
    }
    for (auto gameObject : m_gameObject[l_NOT_TOUMEI]) {
        gameObject->Draw();
    }

    Renderer::SetState_Opaque_B();//ブルームかける不透明

    for (auto gameObject : m_gameObject[l_NOT_TOUMEI_BLOOM]) {
        gameObject->Draw();
    }

    //マテリアルのエミッションを利用してブルームにするとだいぶ内容が変わる
    BloomColor bc;//ブルーム色の定数バッファクリア
    bc.bloomColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    Renderer::UpdateBloomColor(bc);

    ID3D11RenderTargetView* nullRTVs[] = { nullptr, nullptr, nullptr };
    context->OMSetRenderTargets(3, nullRTVs, nullptr);

    // =============================
	// レイマーチング//現在は利用していないが、後々遊びで使うかも
    // =============================
    
    // a. レイマーチング用の設定を呼び出す
    Renderer::SetState_Raymarching();

    // b. レイマーチング用のシェーダーをセット
    context->VSSetShader(Renderer::GetPostEffectVS(), nullptr, 0);
    context->PSSetShader(Renderer::GetRayTestPS(), nullptr, 0);


    // d. 入力として「深度テクスチャ」をシェーダーに渡す
    ID3D11ShaderResourceView* depthSRV = Renderer::GetDepthSRV();
    context->PSSetShaderResources(1, 1, &depthSRV); // t1スロットに深度
  
    //レイマーチング描画
    //Renderer::DrawFullscreenQuad();

    // f. 後片付け
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    context->PSSetShaderResources(1, 1, nullSRV);

    // =============================
	// gbufferパス
    // =============================
    Renderer::SetState_G_Buffer();

    //g-buffer関連
    ID3D11RenderTargetView* litColorRtv = Renderer::GetSceneLitColorRTV();
    context->OMSetRenderTargets(1, &litColorRtv, nullptr);//合成用texture　多分最終これ

    // ライティングシェーダーをセット
    Renderer::GetDeviceContext()->VSSetShader(Renderer::GetCompositeVS(), nullptr, 0);
    context->PSSetShader(Renderer::GetLightingPS(), nullptr, 0);

    // 入力としてG-Bufferのテクスチャ群をセット
    ID3D11ShaderResourceView* gbufferSRVs[] = {
        Renderer::GetSceneColorSRV(), // Albedo
        Renderer::GetSceneNormalSRV(),
        Renderer::GetSceneWorldPosSRV(),
        Renderer::GetShadowMapSRV() // 影
    };
    context->PSSetShaderResources(0, 4, gbufferSRVs);

    //影用設定
    sCamera->Update();
    Renderer::UpdateCsmParams(csmData);

    ID3D11SamplerState* shadowSampler = Renderer::GetShadowSMP();
    context->PSSetSamplers(1, 1, &shadowSampler);

    // フルスクリーンポリゴンを描画
    Renderer::DrawFullscreenQuad();

    // ステート設定 (加算合成, 深度読み取りのみ, 表面カリング)
    Renderer::SetState_PointLight();

	//念のためカメラ行列再設定
    if (camera != nullptr)
    {
        camera->SetMatrix();   
    }

	//ポイントライトレイヤーのオブジェクトを格納
    if (m_drawPointLight)
    {
        for (auto gameObject : m_gameObject[l_LIGHT]) {
            gameObject->Draw();
        }
    }

    //ポイントライト一括描画　インスタンシング
    Renderer::DrawPointLightsInstanced();

    {//かたずけ
        ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 3, nullSRVs);
        Renderer::GetDeviceContext()->OMSetDepthStencilState(Renderer::GetDepthStateEnable(), 0);

        Renderer::GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }
    context->RSSetState(Renderer::GetRsBack());

    //sky描画、前にレイマーチングの関係でこの位置に描画しないといけなかった
    //sky用の描画設定を呼び出す
    Renderer::SetState_SKY();
    for (auto gameObject : m_gameObject[l_SKY]) {
        gameObject->Draw();
    }


	//雲のオブジェクト描画パス============================
    //これ描画先がoit用のRTVじゃないしコスパ悪いから遊びでの使用のみ
    Renderer::SetState_Cloud();
    //カリング無し
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsFront());

    // 雲オブジェクトを描画
    for (auto cloud : m_gameObject[l_CLOUD]) {
        cloud->Draw();
    }

    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());


    // =============================
    //  OIT蓄積パス（半透明オブジェクト）
    // =============================


    // OIT蓄積用の描画設定を呼び出す
    Renderer::SetState_OitAccum();

    // 半透明レイヤーのオブジェクトを描画
    for (auto gameObject : m_gameObject[l_HANTOUMEI]) {
        gameObject->Draw(); // 内部で oitAccum_ps.cso を使用
    }

    // 雲海描画パス============================
    if(m_drawCloud)
    Renderer::DrawVolumetricCloud(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPos());


    // 半透明レイヤーのブルームオブジェクトを描画
    for (auto gameObject : m_gameObject[l_HANTOUMEI_BLOOM]) {
        gameObject->Draw();
    }

    //ブルームバッファクリア
    bc.bloomColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    Renderer::UpdateBloomColor(bc);



    // === OIT合成パス ===
    // OIT合成用の描画設定を呼び出す
    Renderer::SetState_OitComposite();

    Renderer::GetDeviceContext()->VSSetShader(Renderer::GetCompositeVS(), nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(Renderer::GetCompositePS(), nullptr, 0);

    // OIT用の2枚のテクスチャを、ピクセルシェーダーが読み込めるようにセット
    ID3D11ShaderResourceView* srvs[] = { Renderer::GetAccumSRV(), Renderer::GetRevealSRV() };
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 2, srvs);

    // 頂点バッファを使わずにフルスクリーン三角形を描画
    Renderer::GetDeviceContext()->IASetInputLayout(nullptr); // インプットレイアウト不要
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer::GetDeviceContext()->Draw(3, 0);

    // セットしたリソースを解除しておく
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr };
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 2, nullSRVs);

    // パーティクル描画
    //ブルームや描画数を考慮しOITに含めず
    Renderer::SetState_ParticlePMA();
    if (m_drawParticle)
    {
        for (auto gameObject : m_gameObject[l_PARTICLE]) {
            gameObject->Draw();
        }
    }

    //カリング無し
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsNull());

    Renderer::SetState_TransparentEffect();
    for (auto gameObject : m_gameObject[l_GAZOU]) {
        gameObject->Draw();
    }

    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());
    

	

    // =============================
    // ブルーム
    // =============================
    // 1. 描画に必要なシェーダーを取得しておく
    ID3D11VertexShader* vs = Renderer::GetPostEffectVS(); // ポストエフェクト用VS
    ID3D11PixelShader* ps_downsample = Renderer::GetDownsamplePS();
    ID3D11PixelShader* ps_blur = Renderer::GetBlurPS();

	if (m_drawBloom)//ブルーム有効時
    {

        //クリア処理
        for (int i = 0; i < Renderer::bloomLevel; i++)
        {
            Renderer::GetDeviceContext()->ClearRenderTargetView(Renderer::GetDownSampleRTVs(i), bloomClearColor);

        }
        for (int i = 0; i < Renderer::bloomLevel; i++)
        {
            Renderer::GetDeviceContext()->ClearRenderTargetView(Renderer::GetTempRTVs(i), bloomClearColor);
        }
        //ダウンサンプルシェーダーを
        context->VSSetShader(vs, nullptr, 0);
        context->PSSetShader(ps_downsample, nullptr, 0);


        //サンプラー設定
        ID3D11SamplerState* bloomSampler = Renderer::GetBloomSampler();
        Renderer::GetDeviceContext()->PSSetSamplers(0, 1, &bloomSampler);


        {
            for (int i = 0; i < Renderer::bloomLevel; i++)
            {
                //テクセルサイズ
                float inputWidth, inputHeight;

                if (i == 0) {
                    // 初回の入力は「BloomContributionSRV (フルスクリーン)」
                    inputWidth = (float)ScreenSize::ScreenWidth;
                    inputHeight = (float)ScreenSize::ScreenHeight;
                }
                else {
                    // 2回目以降の入力は「一つ前のダウンサンプル結果」
                    // i=1なら入力は(1/2サイズ), i=2なら入力は(1/4サイズ)...
                    int inputDivisor = 1 << i;
                    inputWidth = (float)ScreenSize::ScreenWidth / inputDivisor;
                    inputHeight = (float)ScreenSize::ScreenHeight / inputDivisor;
                }

                //ブルームの定数バッファを利用する
                BloomColor cbData;
                cbData.bloomColor.x = 1.0f / inputWidth;
                cbData.bloomColor.y = 1.0f / inputHeight;
                cbData.bloomColor.z = 0.0f;
                cbData.bloomColor.w = 0.0f;

                Renderer::UpdateBloomColor(cbData);


                //rtvの設定
                ID3D11RenderTargetView* rtv_down = Renderer::GetDownSampleRTVs(i);
                context->OMSetRenderTargets(1, &rtv_down, nullptr);

                // 出力サイズ計算: i=0なら(1/2), i=1なら(1/4)
                int outputDivisor = 2 << i;
                D3D11_VIEWPORT vp_down = {
                    0, 0,
                    (float)ScreenSize::ScreenWidth / outputDivisor,
                    (float)ScreenSize::ScreenHeight / outputDivisor,
                    0, 1
                };
                context->RSSetViewports(1, &vp_down);


                //srvの設定
                ID3D11ShaderResourceView* srv_input = nullptr;
                if (i == 0) {
                    srv_input = Renderer::GetBloomContributionSRV();
                }
                else {
                    srv_input = Renderer::GetDownSampleSRVs(i - 1);
                }
                context->PSSetShaderResources(0, 1, &srv_input);

                //描画
                Renderer::DrawFullscreenQuad();


                //入力に使ったSRVを外す
                ID3D11ShaderResourceView* nullSRV = nullptr;
                context->PSSetShaderResources(0, 1, &nullSRV);
            }
        }



        //ブラー
        context->VSSetShader(vs, nullptr, 0);
        context->PSSetShader(ps_blur, nullptr, 0);
        BlurParams params;
        for (int i = 0; i < Renderer::bloomLevel; i++)
        {

            D3D11_VIEWPORT vp = { 0, 0, ScreenSize::ScreenWidth / (2 << i), (ScreenSize::ScreenHeight / (2 << i)), 0, 1 };
            context->RSSetViewports(1, &vp);
            // 横ブラー
            ID3D11RenderTargetView* rtv_temp = Renderer::GetTempRTVs(i);
            context->OMSetRenderTargets(1, &rtv_temp, nullptr);


            params.blurDirection = { 1.0f, 0.0f };
            params.texelSize = { 1.0f / (ScreenSize::ScreenWidth / (2 << i)), 0.0f };
            Renderer::UpdateBlurParams(params);

            ID3D11ShaderResourceView* srv_down = Renderer::GetDownSampleSRVs(i);
            context->PSSetShaderResources(0, 1, &srv_down);
            Renderer::DrawFullscreenQuad();

        }
        //ブラー
        context->VSSetShader(vs, nullptr, 0);
        context->PSSetShader(ps_blur, nullptr, 0);
        for (int i = 0; i < Renderer::bloomLevel; i++)
        {
            // 縦ブラー
            D3D11_VIEWPORT vp = {
        0, 0,
        (float)ScreenSize::ScreenWidth / (2 << i),
        (float)ScreenSize::ScreenHeight / (2 << i),
        0, 1
            };
            context->RSSetViewports(1, &vp);

            ID3D11RenderTargetView* rtv_down = Renderer::GetDownSampleRTVs(i);
            context->OMSetRenderTargets(1, &rtv_down, nullptr);
            params.blurDirection = { 0.0f, 1.0f };
            params.texelSize = { 0.0f, 1.0f / (ScreenSize::ScreenHeight / (2 << i)) };
            Renderer::UpdateBlurParams(params);


            ID3D11ShaderResourceView* srv_temp = Renderer::GetTempSRVs(i);
            context->PSSetShaderResources(0, 1, &srv_temp);
            Renderer::DrawFullscreenQuad();
        }




        //アップサンプル ---
        {
            float bloomWeights[8] =
            {
                1.0f,   // level 0（最終解像度）
                0.8f,   // 1
                0.6f,   // 2
                0.45f,  // 3
                0.3f,   // 4
                0.2f,   // 5
                0.12f,  // 6
                0.06f   // 7（最小解像度）
            };


            context->OMSetBlendState(Renderer::GetAdditiveBlendState(), nullptr, 0xFFFFFFFF);
            context->PSSetShader(Renderer::GetBloomUpsamplePS(), nullptr, 0);

            // 小さい解像度の結果を、1つ大きい解像度の結果に足しこんでいく
            for (int i = Renderer::bloomLevel - 1; i > 0; i--)
            {
                CompositeParams cp;
                cp.intensityA = bloomWeights[i];
                cp.intensityB = bloomWeights[i];

                Renderer::UpdateCompositeParams(cp);


                // 描画先は、1つ大きい i-1 番目の downsample バッファ
                ID3D11RenderTargetView* rtv = Renderer::GetDownSampleRTVs(i - 1);
                context->OMSetRenderTargets(1, &rtv, nullptr);

                int divisor = 2 << (i - 1);
                D3D11_VIEWPORT vp = { 0, 0, (float)ScreenSize::ScreenWidth / divisor, (float)ScreenSize::ScreenHeight / divisor, 0, 1 };
                context->RSSetViewports(1, &vp);

                // 入力は、1つ小さい i 番目のブラー結果
                ID3D11ShaderResourceView* srv_smaller = Renderer::GetDownSampleSRVs(i);
                context->PSSetShaderResources(0, 1, &srv_smaller);
                Renderer::DrawFullscreenQuad();

            }
            context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        }

    }
        // ===============================================
        // 最終合成
        // ===============================================


            // 1. 描画先をウィンドウ本体（バックバッファ）に戻す
        ID3D11RenderTargetView* backbufferRTV = Renderer::GetRenderTargetView(); // バックバッファ専用のゲッターを用意
        context->OMSetRenderTargets(1, &backbufferRTV, nullptr);
        D3D11_VIEWPORT vp_full = { 0, 0, (float)ScreenSize::ScreenWidth, (float)ScreenSize::ScreenHeight, 0, 1 };
        context->RSSetViewports(1, &vp_full);

        // 2. まず、完成したシーン画像をバックバッファにそのままコピー
        context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF); // ブレンド無効
        context->VSSetShader(vs, nullptr, 0);
        context->PSSetShader(Renderer::GetCopyPS(), nullptr, 0); // 単純なコピー用シェーダー

        ID3D11ShaderResourceView* sceneSRV = Renderer::GetSceneLitColorSRV();
        context->PSSetShaderResources(0, 1, &sceneSRV);
        Renderer::DrawFullscreenQuad();

		/// 3.ブルーム有効の場合の最終合成
        if (m_drawBloom)
        {
            context->OMSetBlendState(Renderer::GetAdditiveBlendState(), nullptr, 0xFFFFFFFF);
            ID3D11ShaderResourceView* finalBloomSRV = Renderer::GetDownSampleSRVs(0);
            context->PSSetShaderResources(0, 1, &finalBloomSRV);
            Renderer::DrawFullscreenQuad();
        }

        // 4. 後片付け
        context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
        context->PSSetShaderResources(0, 2, nullSRVs);
        ID3D11SamplerState* baseSampler = Renderer::GetBaseSampler();
        Renderer::GetDeviceContext()->PSSetSamplers(0, 1, &baseSampler);


        //UIを最後に------------------------------
        Renderer::SetState_UI(); // 深度無効、アルファブレンド有効

        // シェーダーリソースの競合を防ぐため、さっき使ったブルームSRVなどを確実に外しておく
        ID3D11ShaderResourceView* nullSRV_UI[] = { nullptr, nullptr };
        context->PSSetShaderResources(0, 2, nullSRV_UI);
        if (m_drawUI)
        {
            for (auto gameObject : m_gameObject[l_UI]) {
                gameObject->Draw();
            }
        }
        {
//#ifdef _DEBUG

            if (drawImgui)
            {
                // 最初に表示されるときだけ
                ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

                ImGuiManager::Begin(); // ← ImGui開始

                ImGui::Begin("Debug Panel");
                // 1.0 / DeltaTime でFPSが出る
                 // (DeltaTimeが0のときの除算エラー対策で max を使うと安全)
                fpsList[fpsCount % 29] = 1.0f / std::max(Time::DeltaTime(), 0.0001f);

                if (fpsCount % 29 == 0)
                {
                    float sum = 0.0f;
                    for (int i = 0; i < 30; i++)
                    {
                        sum += fpsList[i];
                    }
                    fps = sum / 30.0f;
                }

                fpsCount++;

                ImGui::Text("FPS: %.1f", fps); // 例 "FPS: 60.0"
                ImGui::Text("Frame Time: %.3f ms", Time::DeltaTime() * 1000.0f); // 1フレームにかかった時間(ms)

                static float value[180];
                for (int i = 0; i < 179; i++)
                {
                    value[i] = value[i + 1];

                }

                value[179] = 1.0f / std::max(Time::DeltaTime(), 0.0001f);

				ImGui::PlotLines("Frame Time (ms)", 
                    value, sizeof(value) / sizeof(float), 0, nullptr, 0.0f, 240.0f, ImVec2(0, 80));

                for (int i = 0; i < LAYER_MAX; i++)
                {
                    for (auto gameObject : m_gameObject[i])
                    {
                        gameObject->DrawImgui();
                    }
                }
                ImGui::End();

                ImGui::Begin("Render Target Viewer"); // 新しいウィンドウを開始
                if (ImGui::CollapsingHeader(u8"bloom")) {
                    //ID3D11ShaderResourceView* bloomSrv = Renderer::GetShadowMapSRV();
                    ID3D11ShaderResourceView* bloomSrv = Renderer::GetBloomContributionSRV();

                    if (bloomSrv)
                    {
                        // 2. ImGui::Image() でSRVを画像として表示する
                        //    (void*)へのキャストが必要
                        //    ImVec2(横幅, 高さ)で表示サイズを指定
                        ImGui::Image((void*)bloomSrv, ImVec2(320, 320));
                    }
                    else
                    {
                        ImGui::Text("SRV is null");
                    }

                }
                if (ImGui::CollapsingHeader(u8"ノーマル")) {
                    ID3D11ShaderResourceView* Srv = Renderer::GetSceneNormalSRV();

                    if (Srv)
                    {
                        ImGui::Image((void*)Srv, ImVec2(320, 180));
                    }
                    else
                    {
                        ImGui::Text("SRV is null");
                    }

                }

                if (ImGui::CollapsingHeader(u8"Wpos")) {
                    ID3D11ShaderResourceView* Srv = Renderer::GetSceneWorldPosSRV();

                    if (Srv)
                    {
                        ImGui::Image((void*)Srv, ImVec2(320, 180));
                    }
                    else
                    {
                        ImGui::Text("SRV is null");
                    }

                }
                if (ImGui::CollapsingHeader(u8"汎用")) {
                    ImGuiIO& io = ImGui::GetIO();
                    ID3D11ShaderResourceView* Srv = Renderer::GetSceneColorSRV();

                    if (Srv)
                    {
                        // 2. ImGui::Image() でSRVを画像として表示する
                        //    (void*)へのキャストが必要
                        //    ImVec2(横幅, 高さ)で表示サイズを指定
                        ImGui::Image((void*)Srv, ImVec2(320, 180));
                    }
                    else
                    {
                        ImGui::Text("SRV is null");
                    }

                }

                ImGui::Checkbox("Bloom Enable", &m_drawBloom);
                ImGui::Checkbox("Cloud Enable", &m_drawCloud);
                ImGui::Checkbox("PointLight Enable", &m_drawPointLight);
                ImGui::Checkbox("Particle Enable", &m_drawParticle);
                ImGui::Checkbox("DrawUI", &m_drawUI);

                ImGui::Checkbox("AllObjDataImgui", &GameObject::m_allImgui);

                ImGui::End(); // ウィンドウを閉じる

                //ImGui::ShowDemoWindow();//demo

                ImGuiManager::End();   // ← ImGui描画完了
            }
   
//#endif // DEBUG
    }

}

XMMATRIX CalculateCascadeMatrix(Camera* camera, LIGHT light, const float* cascadeSplits, int cascadeInd,
    XMMATRIX* outMatLightProj,
    XMVECTOR* outLightPos,
    XMVECTOR* outLightForward,
    XMVECTOR* outLightRight,
    XMVECTOR* outLightUp,
    float* outLightFarZ
)
{

    // 1. このカスケード(cascadeInd)が担当する Near と Far の距離を取得
    float cascadeNear = (cascadeInd == 0) ? camera->GetNearClip() : cascadeSplits[cascadeInd];
    float cascadeFar = cascadeSplits[cascadeInd + 1];
    
    // 2. プレイヤーカメラの情報を取得
    XMMATRIX camView = camera->GetViewMatrix();
    float fov = camera->GetFov();
    float aspect = (float)ScreenSize::ScreenWidth / ScreenSize::ScreenHeight;

    // 3. スライスの「近平面」と「遠平面」の縦横の幅を計算
    float tanHalfFovY = tan(fov * 0.5f);
    float tanHalfFovX = tanHalfFovY * aspect;

    float nearY = cascadeNear * tanHalfFovY;
    float nearX = cascadeNear * tanHalfFovX;
    float farY = cascadeFar * tanHalfFovY;
    float farX = cascadeFar * tanHalfFovX;

    // 4. スライスの8つの角を「カメラのビュー空間」で定義
    XMVECTOR cornersView[8];
    cornersView[0] = XMVectorSet(-nearX, nearY, cascadeNear, 1.0f); // Near Top-Left
    cornersView[1] = XMVectorSet(nearX, nearY, cascadeNear, 1.0f); // Near Top-Right
    cornersView[2] = XMVectorSet(nearX, -nearY, cascadeNear, 1.0f); // Near Bottom-Right
    cornersView[3] = XMVectorSet(-nearX, -nearY, cascadeNear, 1.0f); // Near Bottom-Left
    cornersView[4] = XMVectorSet(-farX, farY, cascadeFar, 1.0f);  // Far Top-Left
    cornersView[5] = XMVectorSet(farX, farY, cascadeFar, 1.0f);  // Far Top-Right
    cornersView[6] = XMVectorSet(farX, -farY, cascadeFar, 1.0f);  // Far Bottom-Right
    cornersView[7] = XMVectorSet(-farX, -farY, cascadeFar, 1.0f);  // Far Bottom-Left

    // 5. 8つの角を「ワールド空間」に変換
    XMMATRIX invCamView = XMMatrixInverse(nullptr, camView);
    XMVECTOR cornersWorld[8];
    for (int i = 0; i < 8; ++i)
    {
        cornersWorld[i] = XMVector3TransformCoord(cornersView[i], invCamView);
    }
    // --- ステップ3: 「中心」と「半径」を計算 ---
    XMVECTOR globalCenter = XMVectorZero();
    for (int i = 0; i < 8; ++i) { globalCenter = XMVectorAdd(globalCenter, cornersWorld[i]); }
    globalCenter = XMVectorScale(globalCenter, 1.0f / 8.0f); // 8点の平均

    float maxDistSq = 0.0f;
    for (int i = 0; i < 8; ++i) // プレイヤー視錐台のみ
    {
        maxDistSq = std::max(maxDistSq, XMVectorGetX(XMVector3LengthSq(cornersWorld[i] - globalCenter)));
    }
    float sphereRadius = sqrt(maxDistSq);

    // --- ステップ4: ライトの「View行列」を計算 ---
    XMVECTOR lightDir = XMLoadFloat4(&light.Direction);
    XMVECTOR lightPos = globalCenter - (lightDir * sphereRadius * 2.0f); // 十分後ろに下げる
    XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX matLightView = XMMatrixLookAtLH(lightPos, globalCenter, lightUp);

    // --- ステップ5: 「ライトのビュー空間」に変換 ---
    XMVECTOR cornersLightView[8];
    for (int i = 0; i < 8; ++i)
    {
        cornersLightView[i] = XMVector3TransformCoord(cornersWorld[i], matLightView);
    }

    // --- ステップ6: AABBを計算 ---
    XMVECTOR minVec = cornersLightView[0];
    XMVECTOR maxVec = cornersLightView[0];
    for (int i = 1; i < 8; ++i) // プレイヤー視錐台のみ
    {
        minVec = XMVectorMin(minVec, cornersLightView[i]);
        maxVec = XMVectorMax(maxVec, cornersLightView[i]);
    }

    // --- ステップ7: Projection行列を計算 ---
    float l = XMVectorGetX(minVec);
    float r = XMVectorGetX(maxVec);
    float b = XMVectorGetY(minVec);
    float t = XMVectorGetY(maxVec);
    float n = XMVectorGetZ(minVec);
    float f = XMVectorGetZ(maxVec);

    //深度精度を上げるため、Near/Farを少し調整する
    //例: n -= 100.0f; // 常に手前に余裕を持たせる
    float margin = 100.0f;
    n -= margin;
   
    if (n >= f - 0.01f) {
        n = f - 0.01f;
    }
  
    *outLightFarZ = f;

    XMMATRIX matLightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
    //2. テクセル・スナッピング
    {
        XMMATRIX matLightVP_Unsnapped = matLightView * matLightProj;
        XMVECTOR originClip = XMVector3TransformCoord(XMVectorZero(), matLightVP_Unsnapped);

        float shadowMapSize = (float)Renderer::SHADOW_MAP_SIZE;
        float originTexelX = (originClip.m128_f32[0] * 0.5f + 0.5f) * shadowMapSize;
        float originTexelY = (-originClip.m128_f32[1] * 0.5f + 0.5f) * shadowMapSize;

        float roundedX = roundf(originTexelX);
        float roundedY = roundf(originTexelY);
        float errorX = (roundedX - originTexelX) / shadowMapSize;
        float errorY = (roundedY - originTexelY) / shadowMapSize;

        XMMATRIX matSnap = XMMatrixTranslation(errorX * 2.0f, errorY * 2.0f, 0.0f);
        matLightProj = matLightProj * matSnap;
    }

    *outMatLightProj = matLightProj;
    *outLightPos = lightPos;
    XMVECTOR forward = XMVector3Normalize(globalCenter - lightPos);
    *outLightForward = forward;
    *outLightRight = XMVector3Normalize(XMVector3Cross(lightUp, forward));
    *outLightUp = lightUp;

    return matLightView * matLightProj;

}

