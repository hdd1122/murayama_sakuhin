
#pragma once
#include "gameObject.h"
#include "textsetting.h"


class StrTex : public GameObject
{
    //描画用変数とか
	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11Buffer* m_IndexBuffer{};
	
	static const int MAX_CHARS = 32;//最大文字数

	VERTEX_3D m_Vertex[MAX_CHARS * 4];
	UINT      m_Index[MAX_CHARS * 6];

	UINT                   m_IndexCount=0;

	ID3D11ShaderResourceView* m_Texture;

	//
	class PointLight* m_pointLight = nullptr;//文字を看板として発光する場合のポイントライト
	int m_Frame;
	XMFLOAT3 m_litColor;//ライトの色
	Vector3 transPos;
	bool m_isRot = true;//文字が公転する場合にライトの位置とかが変わるため
	static FontAtlas m_FontAtlas;

	float m_TextWidth = 0.0f;
	float m_TextHeight = 0.0f;

	Vector3 m_ziku;//回転軸（公転用）

	XMFLOAT4 m_backColor;//看板の背景色

	float m_radius = 5.0f;//公転の半径

	float m_style = 1.1f;

	float m_litA = 0.5f;//これブルームに使ってる

	float m_curveRadius = 0.0f;

	float angle;
public:
	void Uninit()override;
	void Init() {}
	//文字、背景色、回転有無、回転軸,曲げる場合
	void Init(const char* text,
		XMFLOAT4 color = XMFLOAT4(0.25, 0.05, 0.08,1.0f), bool isrot = false, 
		Vector3 kaitenziku = Vector3(0, 1, 0),float curveRadius = 0.0f);

	void Update()override;
	void Draw()override;
	void DrawImgui()override;

	//
	void LightSwitch(bool on);//ライトのオンオフ

	void SetLitColor(XMFLOAT3 color);

	void SetRadius(float rad) { m_radius = rad; }
	void SetStyle(float style) { m_style = style; }
	void SetLitA(float a) { m_litA = a; }
};

