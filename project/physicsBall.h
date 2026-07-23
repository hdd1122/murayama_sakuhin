#pragma once
#include "gameobject.h"

class PhysicsBall : public GameObject
{
public:
    Vector3 Velocity;     // 速度 (g_Velocity)
    Vector3 Acceleration; // 加速度 (g_Acceleration)
    float   Mass;         // 質量 (m)
    float   Radius;       // 半径 (r)
    float   BounceFactor; // 跳ね返り係数 (e)

    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    class ModelRenderer* m_modelRenderer;

    void Uninit()override;
    void Init() override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

    // 力を加える（加速させる）
    void AddForce(Vector3 force) { Acceleration += force / Mass; }
};
