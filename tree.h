
#pragma once
#include "gameObject.h"

class Tree : public GameObject
{
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11ShaderResourceView* m_Texture;

	
public:
	void Uninit()override;
	void Init()override;
	void Update()override;
	void Draw()override;

};

