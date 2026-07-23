#pragma once
#include "gameObject.h"

class SelectPoint :public GameObject
{
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11ShaderResourceView* m_Texture;

	static bool m_IsDraw;//これスタティックじゃなくせば汎用性ありそうだけど結局単一だからいいや
	static XMFLOAT2 m_vec;
public:
	void Init() {}
	void Init(float x, float y, float width, float height, const char* fileName);

	void Uninit()override;
	void Update()override;
	void Draw()override;
	static void SetIsDraw(bool draw) { m_IsDraw = draw; }
	static void SetInputVec(XMFLOAT2 vec) { m_vec = vec; }

};

