#pragma once
#include "gameobject.h"

class CheckPoint : public GameObject
{
  
	static constexpr int CheckPointMaxNum = 5;
	//ここにチェックポイントの座標を入れる
    static constexpr XMFLOAT3 cpos[CheckPointMaxNum] = {
        {100.0f, 10.0f, 0.0f},
        {400.0f, 110.0f, 0.0f},
        {400.0f, 110.0f, 400.0f},
        {800.0f, 110.0f, 400.0f},
        {1200.0f, 110.0f, 0.0f},
    };

    XMFLOAT4 m_color = { 1.2f, 1.0f, 1.0f, 1.0f };

	bool m_isActive = true;
	bool m_isGoal = false;

	int m_meNum = 0;//現在のチェックポイント番号

	bool test = false;


    class ModelRenderer* m_modelRenderer{};
    ID3D11VertexShader* m_VertexShader{};
    ID3D11PixelShader* m_PixelShader{};
    ID3D11InputLayout* m_VertexLayout{};



public:
    void Init() {};
    void Init(int CPNum);
    void Uninit()override;
    void Update() override;
    void Draw() override;
    void DrawImgui() override;

};

