#pragma once
#include "gameObject.h"

class Cloud :public GameObject
{

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	class ModelRenderer* m_modelRenderer = nullptr;

	XMFLOAT3 m_noiseOffset; //ノイズのズレ
	float m_density = 0.3f;//0.3が丁度いい
	
public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawImgui()override;


	void SetDensity(float den) { m_density = den; }
};

