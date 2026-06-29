#include "main.h"
#include "manager.h"
#include "PointLight.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "time.h"





void PointLight::Init()
{
	

	m_scale = { 3.0f ,3.0f ,3.0f };
	m_rotation = { 0.0f ,0.0f ,0.0f };
	m_position = { 1.0f ,1.0f ,1.0f };


	m_phase = Renderer::RandomFloat(0.0f, 10.0f);
}
void PointLight::Uninit()
{
	

}

void PointLight::Update()
{


    float time = Time::TotalTime();

    switch (m_type)
    {
    case BehaviorType::GridWave:
    {
        // グリッドの波
        // 基準位置(XZ)に基づいて高さを変える
        float wave = sin(m_basePosition.x * 0.1f + time * m_speed) +
            cos(m_basePosition.z * 0.05f + time * m_speed);

        m_position.y = m_basePosition.y + wave * 0.5f;
        m_position.z = m_basePosition.z + wave * 10.0f;

    }
    break;

    case BehaviorType::Spiral:
    {
        // 螺旋（基準位置を中心に回る）
        // m_phase を「半径」や「高さ」のオフセットとして使うと便利
        float radius = 15.0f;
        float angle = time * m_speed + m_phase;

        m_position.x = cos(angle) * radius - 20;
        m_position.z = sin(angle) * radius;
        // 高さは phase を使って少しずらす
        m_position.y = m_basePosition.y + sin(angle * 2.0f) * 2.0f;
    }
    break;

    case BehaviorType::Highway:
    {
        // 一方向に進んでループ
        float dist = 250.0f;
        // basePosition.z から進んで、一定距離で戻る
        float z = fmod(m_basePosition.z + time * m_speed * 10.0f, dist);
        m_position.z = z - (dist / 2.0f); // 中心合わせ
    }
    break;

    default:
        break;
    }
}

void PointLight::Draw()
{

	
    // データを送るだけでDrawはしない
    PointLightParams data;
    data.LightPos = XMFLOAT3(m_position.x,m_position.y,m_position.z);
    data.LightRange = m_scale.x;
    data.LightColor = m_color;  
    data.LightIntensity = m_intensity;

    // リストに追加
    Renderer::AddPointLight(data);

}
