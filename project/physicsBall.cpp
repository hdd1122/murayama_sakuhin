#include "main.h"
#include "manager.h"
#include "Cube.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "physicsball.h"
#include "time.h"

//タイトル画面用のボール

void PhysicsBall::Init()
{
    m_modelRenderer = new ModelRenderer();
    m_modelRenderer->Load("asset\\ball.obj");

    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_position = Vector3(0.0f, 10.0f, 0.0f);
    Velocity = Vector3(5.0f, 0.0f, 2.0f); // 適当な初速
    Acceleration = Vector3(0.0f, -9.8f, 0.0f); // 重力（授業コードの g_Acceleration）
    Mass = 1.0f;
    Radius = 1.0f;
    BounceFactor = 0.9f; // 反発係数
}

void PhysicsBall::Uninit()
{
    delete m_modelRenderer;

    m_PixelShader->Release();

    m_VertexLayout->Release();
    m_VertexShader->Release();

}
void PhysicsBall::Update()
{
    float dt = Time::GamePlayTime();

    // 速度と位置の更新
    Velocity += Acceleration * dt;

    // 速度制限と強制加速
    float currentSpeed = Velocity.Length();
    float minSpeed = 5.0f;
    float maxSpeed = 15.0f;

    if (currentSpeed < minSpeed && currentSpeed > 0.01f)
    {
        Velocity.Normalize();
        Velocity *= minSpeed;
    }
    else if (currentSpeed > maxSpeed)
    {
        Velocity.Normalize();
        Velocity *= maxSpeed;
    }

    m_position += Velocity * dt;


    // CubeWaveの設定に合わせた定数
    float interval = 1.5f;
    float waveBoundary = 14.0f;

    // 壁の位置
    float wallLimit = waveBoundary - Radius;

    // --- X軸の壁 ---
    if (m_position.x > wallLimit)
    {
        m_position.x = wallLimit;       // 範囲内に押し戻す
        Velocity.x *= -1.0f;            // 速度反転
    }
    else if (m_position.x < -wallLimit)
    {
        m_position.x = -wallLimit;      // 範囲内に押し戻す
        Velocity.x *= -1.0f;            // 速度反転
    }

    // --- Z軸の壁 ---
    if (m_position.z > wallLimit)
    {
        m_position.z = wallLimit;       // 範囲内に押し戻す
        Velocity.z *= -1.0f;            // 速度反転
    }
    else if (m_position.z < -wallLimit)
    {
        m_position.z = -wallLimit;      // 範囲内に押し戻す
        Velocity.z *= -1.0f;            // 速度反転
    }


    // ==========================================
    // 波（床）との当たり判定
    // ==========================================
    
    // --- 波の高さ計算 ---
    float worldDist = sqrtf(m_position.x * m_position.x + m_position.z * m_position.z);

    // グリッド単位に変換 (重要)
    float gridDist = worldDist / interval;

    // 波の高さを計算
    float waveCenterY = 2.0f * sinf(Time::TotalTime() * -2.0f + gridDist * 0.5f);

    // 埋まり防止
    float cubeHalfHeight = 4.0f;

    // 判定ライン
    float groundHeight = waveCenterY + cubeHalfHeight + Radius;

    // 波の表面より下に行ったら
    if (m_position.y < groundHeight)
    {
        m_position.y = groundHeight; // 表面に押し戻す

        // 跳ね返り処理
        Velocity.y *= -1.1f;

        // ランダム反射
        Velocity.x += (rand() % 50 - 25) * 0.02f;
        Velocity.z += (rand() % 50 - 25) * 0.02f;
    }


    // 落下リセット (万が一壁をすり抜けた場合の安全策として残しておく)
    if (m_position.y < -30.0f)
    {
        m_position = Vector3(0.0f, 20.0f, 0.0f);
        Velocity = Vector3((rand() % 5) - 2.5f, 0.0f, (rand() % 5) - 2.5f);
    }
}
void PhysicsBall::Draw()
{
    
        Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

        Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
        Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

        //Renderer::SetWorldViewProjection2D();

        XMMATRIX world, scale, rot, trans;
        scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
        rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
        trans = XMMatrixTranslation(m_position.x, m_position.y+ m_scale.y * 0.5, m_position.z);
        world = scale * rot * trans;

        Renderer::SetWorldMatrix(world);

        BloomColor bc;
        bc.bloomColor = XMFLOAT4(0.5f, 0.1f, 0.5f, 1.0f);
        Renderer::UpdateBloomColor(bc);

        m_modelRenderer->Draw();

    

}


void PhysicsBall::DrawShadow()
{//影用のシェーダー群に
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS(), NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);


    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y + XM_PI, m_rotation.z);
    trans = XMMatrixTranslation(m_position.x, m_position.y + m_scale.y * 0.5, m_position.z);
    world = scale * rot * trans;

    Renderer::SetWorldMatrix(world);

    m_modelRenderer->Draw();

}
