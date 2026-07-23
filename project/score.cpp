#include "main.h"
#include "renderer.h"
#include "Manager.h"
#include "scene.h"
#include "score.h"
#include "StrUI.h"
#include <string>
#include <iomanip>//桁数調整
#include <sstream>//桁数調整

int Score::m_Value;

void Score::Uninit()
{
	
}


void Score::Init()
{
	//UI用設定
	m_scoreUI = Manager::GetScene()->AddGameObject<StrUI>(l_UI);
	m_scoreUI->Init(32);
	m_scoreUI->SetText("Score");
	m_scoreUI->SetPos({ ScreenSize::ScreenWidth * 0.7f, ScreenSize::ScreenHeight * 0.05f, 0.0f });
	m_scoreUI->SetScale({ ScreenSize::ScreenWidth * 0.025f, ScreenSize::ScreenHeight * 0.035f, 1.0f });
	
}

void Score::Update()
{
	//UI更新
	std::ostringstream oss;
	oss << "Score:" << std::setw(5) << std::setfill('0') << m_Value;
	std::string text = oss.str();

	m_scoreUI->SetText(text);
}

void Score::Draw()
{
	
}

void Score::SetPos(Vector3 pos)
{
	m_scoreUI->SetPos(pos);
}

void Score::SetScale(Vector3 scale)
{
	m_scoreUI->SetScale(scale);
}
