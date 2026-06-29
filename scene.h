#pragma once
#include <list>
#include <vector>
#include "gameObject.h"

//
// ポジションは絶対に持っているため、どのような順でオブジェクトが配置されていても絶対に格納自体には失敗しない？
// そもそもオブジェクトの配置は全てコードの外で完了させないと意味がない？
// 
// 
// 
//

//番号変ると面倒な部分あるかもだから後半に追加する
enum LayerName
{
	l_FIRST = 0,		//最初に描画
	l_NOT_TOUMEI,		//不透明オブジェクトの描画
	l_HANTOUMEI,		//半透明
	l_PARTICLE,
	l_GAZOU,			//2dのエフェクトとかー
	l_UI,				//2dのUIとかの描画
	l_test,
	l_SKY,
	l_LIGHT,
	l_CLOUD,
	l_NOT_TOUMEI_BLOOM,
	l_HANTOUMEI_BLOOM,
	OWARI
};

class Scene
{
public:
	//基本関数
	
	virtual ~Scene() {}

	virtual void Init();
	virtual void Update();
	virtual void Draw();
	virtual void Uninit();
	virtual void SaveObj() {}//エラー回避//ゲームシーンでしか使っていない
	virtual void CreateGameObject(std::string str, Vector3 pos, Vector3 scale, Vector3 rot) {}//これエラー回避
	static constexpr int LAYER_MAX = OWARI;
	void SetDrawCloud(bool draw) { m_drawCloud = draw; }
	void SetDrawBloom(bool draw) { m_drawBloom = draw; }
	void SetDrawPointLight(bool draw) { m_drawPointLight = draw; }
	void SetDrawParticle(bool draw) { m_drawParticle = draw; }

private:
	//変数
	std::list<GameObject*> m_gameObject[LAYER_MAX];
	bool m_drawCloud = true;
	bool m_drawBloom = true;
	bool m_drawPointLight = true;
	bool m_drawParticle = true;
	bool m_drawUI = true;

	int objNo = 0;

	//デバック変数
	float fpsList[30] = { 0 };
	int fpsCount = 0;
	float fps = 0.0f;

	bool drawImgui = false;

public:
	//ゲームオブジェクト関連関数

	template <typename T>
	T* AddGameObject(int Layer)
	{
		
		T* gameObject = new T();
		gameObject->Init();
		objNo++;
		gameObject->m_No = objNo;
		m_gameObject[Layer].push_back(gameObject);
		
		return gameObject;
	}

	template <typename T>
	T* GetGameObject()
	{
		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto gameObject : m_gameObject[i])
			{
				T* find = dynamic_cast<T*>(gameObject);//omoiyo-
				if (find != nullptr)
				{
					return find;
				}
			}
		}
		return nullptr;
	}

	template <typename T>
	std::vector<T*> GetGameObjects()
	{
		std::vector < T*> finds;
		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto gameObject : m_gameObject[i])
			{
				T* find = dynamic_cast<T*>(gameObject);//omoiyo-
				if (find != nullptr)
				{
					finds.push_back(find);
				}
			}
		}
		return finds;
	}

	// 全レイヤーの全オブジェクトをまとめて取得する関数
	std::vector<GameObject*> GetAllGameObjects()
	{
		std::vector<GameObject*> allObjects;

		// 全レイヤーをループ
		for (int i = 0; i < LAYER_MAX; i++)
		{
			// そのレイヤーのリストの中身を全部vectorに追加
			for (auto gameObject : m_gameObject[i])
			{
				allObjects.push_back(gameObject);
			}

		}

		return allObjects;
	}

};

