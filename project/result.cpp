#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include"input.h"

#include"title.h"
#include"result.h"
#include"polygon.h"
#include"camera.h"
#include"score.h"
#include"shadowCamera.h"
#include"strUI.h"
#include <iomanip>//桁数調整
#include <sstream>//桁数調整

//リザルトシーン


void Result::Init()
{
	AddGameObject<Camera>(0);
	AddGameObject<ShadowCamera>(0);
	AddGameObject<polygon2D>(l_UI)->Init(0.0f, 0.0f, ScreenSize::ScreenWidth, ScreenSize::ScreenHeight, "asset\\texture\\resultNew.png");
	auto score = AddGameObject<StrUI>(l_UI);
	score->Init(32);
	//std::string scoreText = std::to_string(Score::m_Value);
	std::ostringstream oss;
	oss  << std::setw(5) << std::setfill('0') << Score::m_Value;
	std::string text = oss.str();

	score->SetText(text);
	score->SetPos({ ScreenSize::ScreenWidth * 0.2f, ScreenSize::ScreenHeight * 0.55f,0 });
	score->SetScale({ ScreenSize::ScreenWidth * 0.15f,ScreenSize::ScreenHeight * 0.15f, 1.0f });


	AddGameObject<polygon2D>(l_UI)->Init(ScreenSize::ScreenWidth / 2.0f - (ScreenSize::ScreenWidth / 6.0f * 0.5f),
		ScreenSize::ScreenHeight / 2.0f + 200.0f, ScreenSize::ScreenWidth / 6.0f, ScreenSize::ScreenHeight / 6.0f, "asset\\texture\\enter2.png",true);

	AddGameObject<polygon2D>(l_UI)->Init(ScreenSize::ScreenWidth / 2.0f - (ScreenSize::ScreenWidth / 2.0f * 0.5f),
		ScreenSize::ScreenHeight / 2.0f - 500.0f, ScreenSize::ScreenWidth / 2.0f, ScreenSize::ScreenHeight / 3.0f, "asset\\texture\\Score.png");
	
	//2dレンダリング想定のため雲は描画しない
	SetDrawCloud(false);
}

void Result::Update()
{
	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Score::m_Value = 0;
		Manager::SetScene<Title>();
	}
}
