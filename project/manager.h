#pragma once


class FadeManager
{
	static bool m_isFading;
	static bool m_isFadingOut;
	static float m_fadeTimer;
	static float m_fadeDuration;

	
public:
	static void StartFadeOut(float duration); // フェードアウトを開始
	static void StartFadeIn(float duration);  // フェードインを開始
	static void Update();
	static void Draw(); // フェード用のポリゴンを描画

	static bool IsFading(); // 現在フェード中か

};


class Manager
{
public:
	//static constexpr int LAYER_MAX = 4;
private:
	static class Scene* m_Scene;
	static class Scene* m_SceneNext;
	//static std::list<GameObject*> m_gameObject[LAYER_MAX];


	static bool m_debug;
public:



	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	static Scene* GetScene() { return m_Scene; }
	static const bool& GetDebug() { return m_debug; }

	template <typename T>
	static void SetScene()
	{
		if (m_SceneNext != nullptr) return;

		FadeManager::StartFadeOut(0.5f); // 0.5秒かけてフェードアウト
		m_SceneNext = new T();

	}


};
