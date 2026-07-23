#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "ImGuiManager.h"
#include "Scene.h"
#include "Game.h"
#include "Title.h"
#include "audio.h"
#include "input.h"

#include "polygon.h"
#include "Time.h"
#include "field.h"
#include "camera.h"
#include "player.h"
#include "Result.h"
#include "texture.h"
#include "modelRenderer.h"
#include "animationmodel.h"

#include <algorithm>
//全体管理

Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_SceneNext = nullptr;

bool FadeManager::m_isFading = false;
bool FadeManager::m_isFadingOut = false;
float FadeManager::m_fadeTimer = 0.0f;
float FadeManager::m_fadeDuration = 1.0f;

bool Manager::m_debug = false;



void Manager::Init()
{
	Renderer::Init();
	Input::Init();
	Audio::InitMaster();
	Time::Init();



	m_Scene = new Title();

	m_Scene->Init();

	//
	m_debug = false;
}


void Manager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;



	ModelRenderer::UnloadAll();
	Audio::UninitMaster();
	Input::Uninit();
	Texture::UnloadAll();
	Renderer::Uninit();
}

void Manager::Update()
{
	Time::Update();
	Input::Update();
	m_Scene->Update();

	
#ifdef _DEBUG
	if (Input::GetKeyTrigger('B'))//
	{
		m_debug = !m_debug;
	}
#endif
	FadeManager::Update();

	
}

void Manager::Draw()
{
	Renderer::Begin();


	m_Scene->Draw();

	FadeManager::Draw();

	Renderer::End();


	// フェードアウトが完了した瞬間に、実際のシーン切り替えを行う
	if (m_SceneNext != nullptr && !FadeManager::IsFading()) {

		m_Scene->Uninit();
		delete m_Scene;
	
		
		m_Scene = m_SceneNext;
		m_SceneNext = nullptr;
		m_Scene->Init();

		FadeManager::StartFadeIn(0.5f); // 0.5秒かけてフェードイン
	}
}



void FadeManager::Update()
{
	if (m_isFading) {
		m_fadeTimer += Time::DeltaTime();
	}
}

void FadeManager::Draw()
{
	if (!m_isFading) return;

	// フェードの進捗(0.0～1.0)を計算
	 float progress = std::max(0.0f, std::min(m_fadeTimer / m_fadeDuration, 1.0f));
	// フェードインかアウトかで、アルファ値を決定
	float alpha = m_isFadingOut ? progress : 1.0f - progress;

	// 画面全体を覆う半透明の黒いポリゴンを描画
	Renderer::DrawFullscreenQuad({ 0.0f, 0.0f, 0.0f, alpha });

	// フェードが完了したら終了
	if (progress >= 1.0f) {
		m_isFading = false;
	}
}

void FadeManager::StartFadeOut(float duration)
{
	m_isFading = true;
	m_isFadingOut = true;
	m_fadeTimer = 0.0f;
	m_fadeDuration = duration;
}

void FadeManager::StartFadeIn(float duration)
{
	m_isFading = true;
	m_isFadingOut = false;
	m_fadeTimer = 0.0f;
	m_fadeDuration = duration;
}

bool FadeManager::IsFading()
{
	return m_isFading;
}
