#pragma once
#include "gameObject.h"

class PointLight :public GameObject
{
public:

	enum class BehaviorType {
		None,       // 動かない
		GridWave,   // グリッド状の波
		Spiral,     // 螺旋回転
		Highway     // 直線移動
	};
private:


	XMFLOAT3 m_color;
	float m_intensity;

	XMFLOAT3 m_basePosition; // 基準となる位置
	BehaviorType m_type = BehaviorType::None; // 自分の動きのタイプ
	float m_speed = 1.0f;    // アニメーション速度
	float m_phase = 0.0f;    // 個別のズレ
public:
	void SetColor(XMFLOAT3 c) { m_color = c; }
	void SetIntensity(float i) { m_intensity = i; }

	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;

	void SetBehavior(BehaviorType type, float speed = 1.0f, float phase = 0.0f)
	{
		m_type = type;
		m_speed = speed;
		m_phase = phase;
		m_basePosition = { m_position.x, m_position.y, m_position.z }; // 現在位置を基準にする
	}



};

