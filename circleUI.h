#pragma once
#include "gameObject.h"

class CircleUI :public GameObject
{
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11ShaderResourceView* m_Texture;
	class SelectPoint* m_SelectPoint;

	int m_segments;
	float m_innerRadius;
	float m_quterRadius;
	float m_posX, m_posY;
	int m_vertexCount;

	static bool m_IsDraw;//これスタティックじゃなくせば汎用性ありそうだけど結局使わないからいいや
public:
	void Init() {}
	void Init(float x, float y, float innerRadius, float outerRadius, int segments);

	void Uninit()override;
	void Update()override;
	void Draw()override;
	static void SetIsDraw(bool draw);

};

