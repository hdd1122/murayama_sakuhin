#pragma once
#include "gameObject.h"

class Score :public GameObject
{	
	class StrUI* m_scoreUI = nullptr;
public:
	void Init();
	
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void Add(int value) { m_Value += value; }
	static int m_Value;

	//ちゃんとスコアの中のUIの設定をするため
	void SetPos(Vector3 pos)override;
	void SetScale(Vector3 pos)override;
	
};

