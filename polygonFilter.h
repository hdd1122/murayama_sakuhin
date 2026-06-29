#pragma once
#include "gameObject.h"

class polygonFilter :public GameObject
{
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11ShaderResourceView* m_Texture;
	bool m_move;
public:
	void Init() {}
	void Init(float x, float y, float width, float height, const char* fileName,bool move = false);

	void Uninit()override;
	void Update()override;
	void Draw()override;

};

