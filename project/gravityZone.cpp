#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "gravityZone.h"
#include "player.h"
#include "camera.h"
#include "input.h"
#include "time.h"
#include <algorithm>
#include "scene.h"
#include "circleUi.h"
#include "selectPoint.h"

//半透明重力ゾーン

float GravityZone::m_PZdistance = 10.0f;


// 3Dベクトルを色相(0-360)に変換する関数
float GetHueFromVector(Vector3 dir)
{
    dir.Normalize();
    // XZ平面での角度を計算（真北/Z+ を0度とする例）
    float angle = atan2f(dir.x, dir.z);
    float degree = angle * 180.0f / XM_PI;

    if (degree < 0) degree += 360.0f;
    return degree;
}



//
XMFLOAT4 HSVtoRGB(float h, float s, float v, float a = 1.0f)
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

// 世界座標の3Dベクトルを「XY平面基準」で色に変換する
XMFLOAT4 GetWorldColorFromVector(Vector3 dir)
{
    dir.Normalize();

    // XY平面での角度を計算（右(X+)を0度、上(Y+)を90度とする）
    float hue = atan2f(dir.y, dir.x) * 180.0f / XM_PI;
    if (hue < 0) hue += 360.0f;

    // Z方向の強さに応じて彩度を調整
    // Z方向に突き抜けるほど色が白に近づく
    float depth = fabsf(dir.z);
    float s = 1.0f - (depth * 0.5f);
    float v = 1.0f;

    return HSVtoRGB(hue, s, v, 1.0f);
}

void GravityZone::Init()
{
    m_modelRenderer = new ModelRenderer();
    m_modelRenderer->Load("asset\\cube.obj");
   
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader\\oitAccumPS.cso");

    m_scale = Vector3(3.0f, 3.0f, 3.0f); // サイズ調整
    isZoneForward = false;
    
}
void GravityZone::Uninit()
{

    delete m_modelRenderer;
    m_VertexShader->Release();
    m_PixelShader->Release();
    m_VertexLayout->Release();
}


void GravityZone::Update()
{
    Player* player = Manager::GetScene()->GetGameObject<Player>();
    if (player->GetResetScene())
    {
        SetDestroy();
    }

    if (m_setting)
    {
        Camera* cam = Manager::GetScene()->GetGameObject<Camera>();

        // 時間に応じて長さを更新
        m_chargeTime += Time::DeltaTime();
        float length = std::min(m_chargeTime * 5.0f, m_maxLength);
        m_scale = Vector3(3.0f, length, length);

       
        // BeginSettingで記録した向きと開始位置を元に、ゾーンの位置を更新
        Vector3 offset = GetForward() * (length * 0.5f);

        if(player->GetZoneCreatePPos())
        m_position = player->GetPos() + offset;
        else//遠隔モード
        m_position = player->GetPos() + cam->GetForward() * m_PZdistance + offset;

       
        // マウスの現在位置を取得
        POINT cursorPos = MouseInput::GetLogicalCursorPos();

        // UIの中心からマウスカーソルまでのベクトルを計算
        XMFLOAT2 direction;
        direction.x = (float)cursorPos.x - m_uiCenter.x;
        direction.y = (float)cursorPos.y - m_uiCenter.y;

        SelectPoint::SetInputVec(direction);

        //ベクトル
        Vector3 inVec = { direction.x, direction.y, 0.0f };
        float inLen = inVec.Length();
        inVec.Normalize();
        m_direction = inVec; // 2D平面上の正規化入力

        
        Vector3 currentWorldDir = (cam->GetRight() * m_direction.x) + (cam->GetUp() * -m_direction.y);
        currentWorldDir.Normalize();

        // 正面重力のときの世界ベクトル
        Vector3 forwardWorldDir = GetForward();

   
        m_color = GetWorldColorFromVector(currentWorldDir);
        m_color.w = 0.1f;
        isZoneForward = false;

        // 中央付近の判定、付近は正面
        if (inLen <= 50.0f)
        {
            m_color = GetWorldColorFromVector(forwardWorldDir);
            m_color.w = 0.1f;
            isZoneForward = true;
            m_scale = Vector3(3.0f, 3.0f, length);

        }



        const int lenMax = 450;
        //マウス操作の快適性を求めた原点の移動
        if (inLen > lenMax)
        {
            inVec *= inLen - lenMax;
          
            m_uiCenter.x += inVec.x;
            m_uiCenter.y += inVec.y;
        }


        //ゾーン設置確定
        if (Input::GetKeyPress(VK_LBUTTON) || Input::GetKeyPress('G'))
        {
            m_gravityDirection = isZoneForward ? forwardWorldDir : currentWorldDir;
            m_color = GetWorldColorFromVector(m_gravityDirection);
            m_color.w = 0.25f;
            m_setting = false;
            CircleUI::SetIsDraw(false);
            Camera::SetIsMove(true);
            Time::SetTimeScale(1.0f);
            player->AddZoneCount();
            return;
        }

        // 入力解除
        if (!Input::GetKeyPress(VK_RBUTTON))
        {
            //UI非表示
            CircleUI::SetIsDraw(false);
            Camera::SetIsMove(true);
            //通常状態に戻す
            Time::SetTimeScale(1.0f);
            SetDestroy();
            return;
        }



    }//setting 終了


    // OBBの情報を更新する
    m_obb.Center = m_position;
    m_obb.Extents = m_scale;

    XMMATRIX rotMatrix;

    // 1. UIで決めた重力方向(m_direction)から「追加の回転」を計算
    XMFLOAT3 buffer = { m_direction.x, m_direction.y, m_direction.z };
    XMVECTOR planeNormal = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    XMVECTOR targetDirection = XMLoadFloat3(&buffer);

    targetDirection = XMVectorSetX(targetDirection, -XMVectorGetX(targetDirection));

    XMVECTOR rotationQuaternion;
    // 2つのベクトル間の回転を計算
    XMVECTOR rotationAxis = XMVector3Cross(planeNormal, targetDirection);
    if (XMVectorGetX(XMVector3LengthSq(rotationAxis)) < 1e-6)
    {
        // 2つのベクトルが平行な場合は、回転なし
        rotationQuaternion = XMQuaternionIdentity();
    }
    else
    {
        float angle = acosf(XMVectorGetX(XMVector3Dot(planeNormal, targetDirection)));
        rotationQuaternion = XMQuaternionRotationAxis(rotationAxis, angle);
    }

    // クォータニオンから行列を作成
    XMMATRIX rot_from_ui = XMMatrixRotationQuaternion(rotationQuaternion);

    // BeginSettingで保存したカメラの回転から基準の回転を計算
    XMMATRIX rot_from_camera = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

    // 2つの回転行列を合成
    rotMatrix = rot_from_ui * rot_from_camera;

    //正面
    if (isZoneForward)
        rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

    XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);
}


void GravityZone::Draw()
{

    // --- 共通設定 ---
    // シェーダーとインプットレイアウトはここでセットする
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    // ワールド行列の計算とセット
    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

    // 1. UIで決めた重力方向(m_direction)から「追加の回転」を計算
    XMFLOAT3 buffer = { m_direction.x, m_direction.y, m_direction.z };
    XMVECTOR planeNormal = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    XMVECTOR targetDirection = XMLoadFloat3(&buffer);

    targetDirection = XMVectorSetX(targetDirection, -XMVectorGetX(targetDirection));

    XMVECTOR rotationQuaternion;
    // 2つのベクトル間の回転を計算
    XMVECTOR rotationAxis = XMVector3Cross(planeNormal, targetDirection);
    if (XMVectorGetX(XMVector3LengthSq(rotationAxis)) < 1e-6)
    {
        // 2つのベクトルが平行な場合は、回転なし
        rotationQuaternion = XMQuaternionIdentity();
    }
    else
    {
        float angle = acosf(XMVectorGetX(XMVector3Dot(planeNormal, targetDirection)));
        rotationQuaternion = XMQuaternionRotationAxis(rotationAxis, angle);
    }

    // クォータニオンから行列を作成
    XMMATRIX rot_from_ui = XMMatrixRotationQuaternion(rotationQuaternion);

    // BeginSettingで保存したカメラの回転から「基準の回転」を計算
    XMMATRIX rot_from_camera = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

    // 2つの回転行列を合成
    rot = rot_from_ui * rot_from_camera;

    //正面
    if(isZoneForward)
        rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

    trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    world = scale * rot * trans;
    Renderer::SetWorldMatrix(world);


    // --- 外壁の描画 ---
    // ラスタライザーステートを裏面カリングに設定
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());

    // 外壁用のマテリアル（薄めの色）を設定
    MATERIAL materialOuter = {};
    materialOuter.Diffuse = m_color;
    Renderer::SetMaterial(materialOuter);

    // 描画
    m_modelRenderer->DrawOitAccumulation();


    // --- 内壁の描画 ---
    // ラスタライザーステートを表面カリングに切り替える8
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsFront());

    // 内壁用のマテリアル（濃いめの色）を設定
    MATERIAL materialInner = {};
    materialInner.Diffuse.x = materialOuter.Diffuse.x * 1.25f;
    materialInner.Diffuse.y = materialOuter.Diffuse.y * 1.25f;
    materialInner.Diffuse.z = materialOuter.Diffuse.z * 1.25f;
    materialInner.Diffuse.w = materialOuter.Diffuse.w * 1.25f;
    Renderer::SetMaterial(materialInner);

    // 再度、同じモデルを描画
    m_modelRenderer->DrawOitAccumulation();
    // 他のオブジェクトに影響を与えないよう、ラスタライザーステートを通常の状態に戻す
    Renderer::GetDeviceContext()->RSSetState(Renderer::GetRsBack());

   
}




void GravityZone::BeginSetting()
{
    m_setting = true;
    m_chargeTime = 0.0f;

    //右クリックを押しはじめた地点がUI原点
    POINT cursorPos = MouseInput::GetLogicalCursorPos();
    m_uiCenter.x = cursorPos.x;
    m_uiCenter.y = cursorPos.y;

    // 視線方向と現在位置を記録
    Camera* cam = Manager::GetScene()->GetGameObject<Camera>();
    if (cam)
    {
        m_rotation = cam->GetRot();
    }
}
