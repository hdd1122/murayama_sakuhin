#pragma once
#include "gameObject.h"
#include "textsetting.h"
#include <string>

class StrUI : public GameObject
{
    //描画用変数とか
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	

	ID3D11Buffer* m_IndexBuffer{};
	
	static const int MAX_CHARS = 32;//最大文字数

	ID3D11ShaderResourceView* m_Texture;

	static FontAtlas m_FontAtlas;

	XMFLOAT4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::string m_CurrentText;
	int m_MaxChars = 0;
	int m_CurrentCharCount = 0;
public:
	void Uninit()override;
	void Init() {}
	//
	void Init(int maxChars);//半角文字数

	void Update()override;
	void Draw()override;
	void DrawImgui()override;


	void SetText(const std::string& text);
};

