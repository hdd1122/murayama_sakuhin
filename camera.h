#pragma once
#include "gameObject.h"

class Camera : public GameObject
{
	XMMATRIX m_projection;
	XMMATRIX m_view;
	float m_nearClip = 0.1f;
	float m_farClip = 1000.0f;
	float m_fov = 1.0f;//これ57度くらい

	Vector3 m_Target{0.0f,0.0f,0.0f};
	
	static bool m_isMove;
   
	float m_moveSpeed = 0.5f;//デバック時カメラ移動
	float m_rotSpeed = 0.003f;//デバック時カメラ移動
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

	void SetFovDegree(float deg)
	{
		m_fov = XMConvertToRadians(deg);
	}

	XMMATRIX GetProjectionMatrix() { return m_projection; }

	XMMATRIX GetViewMatrix() { return m_view; }
	void SetMatrix();
	static void SetIsMove(bool move) { m_isMove = move; }

	float GetNearClip() { return m_nearClip; }
	float GetFarClip() { return m_farClip; }
	float GetFov() { return m_fov; }




    // 平面を表す構造体
    struct Plane
    {
        XMFLOAT3 Normal; // 法線（壁の向き）
        float    Distance; // 原点からの距離

        // 正規化（計算を正しく行うために必須）
        void Normalize()
        {
            float length = sqrtf(Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z);
            if (length > 0.0f)
            {
                float invLength = 1.0f / length;
                Normal.x *= invLength;
                Normal.y *= invLength;
                Normal.z *= invLength;
                Distance *= invLength;
            }
        }
    };

    // カリング用クラス
    class ViewFrustum
    {
    public:
        Plane m_Planes[6]; // 上下左右前後 6枚の壁

        // カメラの View行列 と Projection行列 から6枚の壁を作る関数
        // ※これを毎フレーム 1回だけ呼ぶ
        void Update(XMMATRIX viewMatrix, XMMATRIX projMatrix)
        {
            // View * Proj 行列を作る
            XMMATRIX vp = XMMatrixMultiply(viewMatrix, projMatrix);

            // DirectXMathの行列要素にアクセスしやすいように格納
            XMFLOAT4X4 m;
            XMStoreFloat4x4(&m, vp);

            // 6枚の平面を抽出（Gribb-Hartmann法という定番アルゴリズム）

            // Left Plane
            m_Planes[0].Normal.x = m._14 + m._11;
            m_Planes[0].Normal.y = m._24 + m._21;
            m_Planes[0].Normal.z = m._34 + m._31;
            m_Planes[0].Distance = m._44 + m._41;

            // Right Plane
            m_Planes[1].Normal.x = m._14 - m._11;
            m_Planes[1].Normal.y = m._24 - m._21;
            m_Planes[1].Normal.z = m._34 - m._31;
            m_Planes[1].Distance = m._44 - m._41;

            // Bottom Plane
            m_Planes[2].Normal.x = m._14 + m._12;
            m_Planes[2].Normal.y = m._24 + m._22;
            m_Planes[2].Normal.z = m._34 + m._32;
            m_Planes[2].Distance = m._44 + m._42;

            // Top Plane
            m_Planes[3].Normal.x = m._14 - m._12;
            m_Planes[3].Normal.y = m._24 - m._22;
            m_Planes[3].Normal.z = m._34 - m._32;
            m_Planes[3].Distance = m._44 - m._42;

            // Near Plane
            m_Planes[4].Normal.x = m._13;
            m_Planes[4].Normal.y = m._23;
            m_Planes[4].Normal.z = m._33;
            m_Planes[4].Distance = m._43;

            // Far Plane
            m_Planes[5].Normal.x = m._14 - m._13;
            m_Planes[5].Normal.y = m._24 - m._23;
            m_Planes[5].Normal.z = m._34 - m._33;
            m_Planes[5].Distance = m._44 - m._43;

            // 3. 全ての平面を正規化（重要！）
            for (int i = 0; i < 6; i++)
            {
                m_Planes[i].Normalize();
            }
        }

        // objPos: objの中心座標
        // radius: obj球の半径
        bool IsVisible(Vector3 objPos, float radius)
        {
            for (int i = 0; i < 6; i++)
            {
                // 点と平面の距離を計算
                float dist = (m_Planes[i].Normal.x * objPos.x) +
                    (m_Planes[i].Normal.y * objPos.y) +
                    (m_Planes[i].Normal.z * objPos.z) +
                    m_Planes[i].Distance;

                // 半径分よりも-あったら見えない
                if (dist < -radius)
                {
                    return false; // 1枚でも裏側にあったらアウト
                }
            }
            return true; // 全ての壁の内側（または交差）なら見える
        }
    };

    private:
     ViewFrustum m_Frustum; // 視錐台変数

     public:
         // 他が判定機を使えるようにするゲッター
         ViewFrustum& GetFrustum() { return m_Frustum; }
};

