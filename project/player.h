#pragma once
#include "gameObject.h"
#include <string>


class Player:public GameObject
{

	//現段階でプレイヤーがシーンをまたいで持ち越されることは無い
	//よっていまはここで初期化でもまあ
	//ただ、initを明示的に呼ぶ場合とかがあると変更必須

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	//class ModelRenderer* m_modelRenderer;
	class AnimationModel* m_animationModel;

	//ゾーン設定用UI
	class CircleUI* circleUi = nullptr;

	class StrUI* testUI = nullptr;

	//空中でのゾーン生成回数制限
	//UIは 1/10みたいな数字
	const int MAX_ZONE_COUNT = 6;
	int m_zoneCount = 0;

	Vector3 m_velocity{};
	Vector3 m_gravityVelocity{};
	float m_gravity;

	Vector3 m_acc{};

	bool m_isGround;
	float m_Frame;

	std::string m_AnimationName;
	std::string m_AnimationNameNext;
	float m_AnimationBlend;

	class Audio* m_SE;//コイン

	Vector3 m_resetPos{};			//リスポーン地点

	//fov
	float m_fov = 60.0f;//現在

	float m_baseFov = 60.0f;		// 通常時のFOV
	float m_maxFov = 80.0f;			// ゾーン加速時の最大FOV
	float m_currentFov = 70.0f;		// 現在のFOV
	float m_fovResetTimer = 0.0f;	// FOVを戻すまでのタイマー
	float m_fovChangeSpeed = 5.0f;	// FOVが変化する速さ
	
	bool m_zoneCreatePPos = true;//ゾーンの生成位置をプレイヤーの位置にするかどうか

	bool m_resetScene = false;
public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawShadow()override;
	void DrawImgui()override;

	Vector3 GetVel() { return m_velocity; }

	void SetResetPos(Vector3 pos) { m_resetPos = pos; }

	int sceneNo = 1;
	bool isFirst;
	bool m_changeScene = false;

	void AddZoneCount()
	{
		m_zoneCount++;
	}

	void CreateZoneUI();

	bool GetZoneCreatePPos() { return m_zoneCreatePPos; }
	bool GetResetScene() {return m_resetScene;}

};

