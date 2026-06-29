#pragma once


#include "gameObject.h"

class MeshField : public GameObject
{

private:
	ID3D11Buffer*				m_VertexBuffer{};
	ID3D11Buffer*				m_IndexBuffer{};
	ID3D11ShaderResourceView*	m_Texture{};
	ID3D11ShaderResourceView*	m_TextureSand{};

	VERTEX_3D					m_Vertex[21][21]{};

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	float GetHeight(Vector3 Position);
	void SaveY();
	void reLoad();

};
