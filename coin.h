#pragma once
#include "gameObject.h"

class Coin :public GameObject
{

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	class ModelRenderer* m_modelRenderer = nullptr;
	class PointLight* m_pointLight;
public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawShadow()override;
	void DrawImgui()override;

	std::string GetName() override
	{
		return "Coin";
	}
};

