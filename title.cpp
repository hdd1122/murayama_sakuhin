#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include"input.h"

#include"title.h"
#include"game.h"
#include"polygon.h"
#include"circleUI.h"
#include"camera.h"
#include"player.h"
#include"sky.h"
#include"realityFragment.h"
#include"ShadowCamera.h"
#include"cubewave.h"
#include"PhysicsBall.h"
#include"strTex.h"

//タイトル画面


void Title::Init()
{
	MouseInput::SetUIMode(true);
	AddGameObject<Camera>(0);
	AddGameObject<ShadowCamera>(0);
	//AddGameObject<polygon2D>(UI)->Init(0.0f, 0.0f, ScreenSize::ScreenWidth, ScreenSize::ScreenHeight, "asset\\texture\\setumei.png");
	AddGameObject<RealityFragment>(3)->Init(20000, "asset\\texture\\sakura.png");
	AddGameObject<Sky>(l_SKY)->SetScale({ 200.0f,200.0f,200.0f });

	auto player = AddGameObject<Player>(1);
	player->sceneNo = 0;

	AddGameObject<CubeWave>(1);

	// Scene::Init()
	auto m_Ball = AddGameObject<PhysicsBall>(l_NOT_TOUMEI_BLOOM);
	m_Ball->SetPos(Vector3(-5.0f, 5.0f, 0.0f));
	m_Ball->Velocity = Vector3(3.0f, 0.0f, 0.0f); // 右へ
	m_Ball = AddGameObject<PhysicsBall>(l_NOT_TOUMEI_BLOOM);
	m_Ball->SetPos(Vector3(5.0f, 5.0f, 0.0f));
	m_Ball->Velocity = Vector3(-3.0f, 0.0f, 0.0f); // 左へ
	
	auto testChar = AddGameObject<StrTex>(l_GAZOU);
	testChar->Init("DXTITLE", XMFLOAT4(0.25, 0.05, 0.5, 1.1f), false, Vector3(0, 1, 0));
	testChar->SetScale({6,4,1});
	testChar->LightSwitch(true);
	testChar->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar->SetPos({ -16.0f, 15.0f, 3.0f });
	testChar->SetRadius(2.0f);


	
	AddGameObject<polygon2D>(l_UI)->Init(ScreenSize::ScreenWidth / 2.0f - (ScreenSize::ScreenWidth / 6.0f * 0.5f),
		ScreenSize::ScreenHeight / 2.0f + 200.0f, ScreenSize::ScreenWidth / 6.0f, ScreenSize::ScreenHeight / 6.0f, "asset\\texture\\enter2.png",true);

}

void Title::Update()
{
	Scene::Update();
	if (Input::GetKeyTrigger(VK_RETURN)) 
	{
		Manager::SetScene<Game>();
	}
}
