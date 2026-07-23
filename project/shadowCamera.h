#pragma once
#include "gameObject.h"

class ShadowCamera : public GameObject
{
	XMMATRIX m_projection;
	XMMATRIX m_view;

	Vector3 m_Target{0.0f,0.0f,0.0f};
	LIGHT m_light{};

public:
	void Uninit() override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawImgui()override;

	Vector3 GetForward()override
	{
		Vector3 forward = m_Target - m_position;
		forward.Normalize();
		return forward;
	}

	XMMATRIX GetProjectionMatrix() { return m_projection; }

	XMMATRIX GetViewMatrix() { return m_view; }
	LIGHT GetLight(){ return m_light; }
	void SetMatrix();
};

