#pragma once
#include "gameObject.h"
#include <string>


class GravityBall:public GameObject
{

	class ModelRenderer* m_modelRenderer{};

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	Vector3 m_velocity{};
	Vector3 m_gravityVelocity{};
	float m_gravity;

	Vector3 m_acc{};

	bool m_isGround;
	float m_Frame;

	class Audio* m_SE;//コイン

	Vector3 m_resetPos{};			//リスポーン地点

	int m_goalNum = 0;

public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawShadow()override;
	void DrawImgui()override;

	Vector3 GetVel() { return m_velocity; }

	void SetResetPos(Vector3 pos) { m_resetPos = pos; }

	void SetGoalNum(int num) { m_goalNum=num; }
	int sceneNo = 1;
	bool isFirst;
	bool m_changeScene = false;



};

