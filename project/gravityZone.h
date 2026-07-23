#pragma once
#include "zone.h"


class GravityZone : public Zone
{
    bool m_setting = false;          // 設置中かどうか
    float m_chargeTime = 0.0f;       // 入力時間
    float m_maxLength = 10.0f;       // 最大長さ
   
    bool isZoneForward = false;
    Vector3 m_gravityDirection;//
  
    XMFLOAT4 m_color = { 1.2f, 1.0f, 1.0f, 1.0f };


    XMFLOAT2 m_uiCenter;

    ModelRenderer* m_modelRenderer{};
    ID3D11VertexShader* m_VertexShader{};
    ID3D11PixelShader* m_PixelShader{};
    ID3D11InputLayout* m_VertexLayout{};

    
	static float m_PZdistance; // プレイヤーとゾーンの距離


public:
    void BeginSetting();
    void Init() override;
    void Uninit()override;
    void Update() override;
    void Draw() override;

    Vector3 GetGravityDirection() const { return m_gravityDirection; }

    bool GetSetting() { return m_setting; }

    //基本こっちで設定
    static void AddPZdistance(float num) {
        m_PZdistance += num;
        //範囲制限
		m_PZdistance = std::max(1.0f, std::min(m_PZdistance, 40.0f));
    }
    //切り替え時のリセットとか
    static void SetPZdistance(float distance) {
		m_PZdistance = distance;
		//範囲制限
		m_PZdistance = std::max(1.0f, std::min(m_PZdistance, 40.0f));
	}


};

