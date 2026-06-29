#pragma once
#include "gameObject.h"



class ParticleSystem : public GameObject
{
private:
    // コンピュートシェーダー関連
    ID3D11ComputeShader* m_ComputeShader = nullptr;
    ID3D11Buffer* m_ParticleBufferA = nullptr;
    ID3D11Buffer* m_ParticleBufferB = nullptr;
    ID3D11ShaderResourceView* m_ParticleSRVA = nullptr;
    ID3D11ShaderResourceView* m_ParticleSRVB = nullptr;
    ID3D11UnorderedAccessView* m_ParticleUAVA = nullptr;
    ID3D11UnorderedAccessView* m_ParticleUAVB = nullptr;
    bool m_PingPong = false;

    // 描画シェーダー関連
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11ShaderResourceView* m_Texture = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    // インスタンシング用の頂点バッファ
    ID3D11Buffer* m_QuadVertexBuffer = nullptr;
    ID3D11Buffer* m_QuadIndexBuffer = nullptr;
  
   

    UINT m_MaxParticles;

public:
    void Init() {}
    void Init(UINT maxParticles, const char* textureFile);
    void Uninit() override;
    void Update() override; // コンピュートシェーダーを実行
    void Draw() override;   // パーティクルを描画
};
