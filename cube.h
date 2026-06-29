#pragma once
#include "gameObject.h"

class Cube :public GameObject
{

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	class ModelRenderer* m_modelRenderer = nullptr;
	Vector3 m_color = {1, 1, 1};
public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;
	void DrawShadow()override;
	void SetColor(Vector3 color) { m_color = color; }
};

