#pragma once
#include "obb.h"
#include <string>

class GameObject
{
protected:
	Vector3 m_position{0.0f,0.0f,0.0f};
	Vector3 m_rotation{ 0.0f,0.0f,0.0f };
	Vector3 m_scale{ 1.0f,1.0f,1.0f };

	bool m_isHighlight = false;

	Obb m_obb;
	float m_radius = 1.0;

	bool m_destroy = false;
	XMMATRIX m_world;//影などの再描画時に使うかも

	bool m_isSaveObj = false;//ファイルにセーブするか？
	bool m_isCreateObj = false;	//コード以外で生成されたオブジェクトか？


	
public:
	virtual ~GameObject() {}
	virtual void Init() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Uninit() {}
	virtual void DrawImgui();

	virtual void DrawShadow() {};

	static bool m_allImgui;
		
	void SetDestroy() { m_destroy = true; }
	bool GetDestroy() { return m_destroy; }
	bool Destroy()
	{
		if (m_destroy && !m_isSaveObj)
		{
			Uninit();
			delete this;
			return true;
		}
		else
		{
			return false;
		}

	}

	Vector3 GetPos() const { return m_position; }
	Vector3 GetRot() const { return m_rotation; }
	Vector3 GetScale() const { return m_scale; }
	virtual void SetPos(Vector3 pos) { m_position = pos; }
	void SetRot(Vector3 rot) { m_rotation = rot; }
	virtual void SetScale(Vector3 scale) { 
		m_scale = scale;
		m_radius = scale.x;
	}

	Vector3 GetRight()
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	
		Vector3 right;
		XMStoreFloat3((XMFLOAT3*)&right, matrix.r[0]);//行列０行目
		return right;
	}
	virtual Vector3 GetForward()
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, matrix.r[2]);//行列2行目z
		return forward;
	}

	Vector3 GetUp()
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

		Vector3 up;
		XMStoreFloat3((XMFLOAT3*)&up, matrix.r[1]); // 行列の1行目(Y)を取得
		return up;
	}
	float GetDistance(Vector3 pos)
	{
		return (m_position - pos).Length();
	}

	float GetZ(Vector3 pos,Vector3 forward)
	{
		//return Vector3::Dot((m_position - pos), forward);
		return (m_position - pos).LengthSq();
		//return 1.0f;
	}


	const Obb& GetObb() const { return m_obb; }
	float GetRadius() { return m_radius; }

	int m_No = 0;

	void SetSaveObj(bool isSave) { m_isSaveObj = isSave; }
	bool GetSaveObj() { return m_isSaveObj; }

	void SetCreateObj(bool isSave) { m_isCreateObj = isSave; }
	bool GetCreateObj() { return m_isCreateObj; }



	//自分のクラス名を返す関数　生成時に使う
	virtual std::string GetName()
	{
		return "Unknown"; // デフォルトの名前
	}
};

