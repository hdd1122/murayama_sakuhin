#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "audio.h"

#include "game.h"
#include "camera.h"
#include "shadowcamera.h"
#include "field.h"
#include "time.h"
#include "player.h"
#include "polygon.h"
#include "input.h"
#include "result.h"
#include "score.h"
#include "sky.h"
#include "GroundBox.h"
#include "meshField.h"
#include "particle.h"
#include "realityFragment.h"
#include "coin.h"
#include "wave.h"
#include "tree.h"
#include "Grass.h"
#include "Rock.h"
#include "PointLight.h"
#include "cloud.h"
#include "Building.h"
#include "StrTex.h"
#include "CheckPoint.h"
#include "PointObject.h"
#include "StrUI.h"
#include "GravityBall.h"
#include "ShootArea.h"

# include <fstream>
# include <sstream>


XMFLOAT3 HsvToRgb(float h, float s, float v)
{
	float c = v * s;
	float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
	float m = v - c;
	float r = 0, g = 0, b = 0;
	if (h < 60) { r = c; g = x; b = 0; }
	else if (h < 120) { r = x; g = c; b = 0; }
	else if (h < 180) { r = 0; g = c; b = x; }
	else if (h < 240) { r = 0; g = x; b = c; }
	else if (h < 300) { r = x; g = 0; b = c; }
	else { r = c; g = 0; b = x; }
	return { r + m, g + m, b + m };
}



//これ一旦ゲームシーンの個別機能に
void Game::CreateGameObject(std::string str, Vector3 pos, Vector3 scale, Vector3 rot)
{

	//ここに生成したいオブジェクトのクラス名と生成関数を追加していく
	//ルールとして、下のようにクラス名を返す関数(GetName)をクラスごとに.hに追加する

	//コインの場合
	//std::string GetName() override
	//{
	//	return "Coin";
	//}

	//ゲーム内生成のためimguiのボタンで呼び出す予定、カメラ位置に生成
	//カメラのimguiとかで呼ぶ

	//オブジェクトのimguiも変更する
	//カメラの生成部分にもオブジェクト名を追加する

	//現在は初期化が単純なもののみ

	if (str == "PointObject")
	{
		PointObject* obj = AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM);
		obj->SetPos(pos);
		obj->SetScale(scale);
		obj->SetRot(rot);
		obj->SetSaveObj(true);
		obj->SetCreateObj(true);

	}
	else if (str == "Coin")
	{
		Coin* obj = AddGameObject<Coin>(l_NOT_TOUMEI_BLOOM);
		obj->SetPos(pos);
		obj->SetScale(scale);
		obj->SetRot(rot);
		obj->SetSaveObj(true);
		obj->SetCreateObj(true);
	}
	else if (str == "GroundBox")
	{
		GroundBox* obj = AddGameObject<GroundBox>(l_NOT_TOUMEI);
		obj->SetPos(pos);
		obj->SetScale(scale);
		obj->SetRot(rot);
		obj->SetSaveObj(true);
		obj->SetCreateObj(true);
	}

}

void Game::LoadObj()
{
	std::string str_line_buf;
	std::string input_csv_file_path = "gameObjData.csv";

	// ファイルを開く
	std::ifstream ifs_csv_file(input_csv_file_path);

	// ファイルが開けなかった場合のエラーチェック
	if (!ifs_csv_file.is_open()) {
		return;
	}

	// 1行ずつ読み込む loop
	while (getline(ifs_csv_file, str_line_buf))
	{
		// 空行なら飛ばす
		if (str_line_buf.empty()) continue;

		std::istringstream i_stream(str_line_buf);
		std::string str_token;

		//名前
		std::string name;
		if (!getline(i_stream, name, ',')) continue; // 読み込めなければ次へ

		//position
		//x
		std::string px_str;
		if (!getline(i_stream, px_str, ',')) continue;

		//y
		std::string py_str;
		if (!getline(i_stream, py_str, ',')) continue;

		//z
		std::string pz_str;
		if (!getline(i_stream, pz_str, ',')) continue;

		//scale
		//x
		std::string sx_str;
		if (!getline(i_stream, sx_str, ',')) continue;

		//y
		std::string sy_str;
		if (!getline(i_stream, sy_str, ',')) continue;

		//z
		std::string sz_str;
		if (!getline(i_stream, sz_str, ',')) continue;

		//rotation
		//x
		std::string rx_str;
		if (!getline(i_stream, rx_str, ',')) continue;

		//y
		std::string ry_str;
		if (!getline(i_stream, ry_str, ',')) continue;

		//z
		std::string rz_str;
		getline(i_stream, rz_str, ','); // 最後はカンマがない可能性もあるのでifチェックは緩め

		// 文字列を数値floatに変換
		try {//エラーの可能性があるため
			float px = std::stof(px_str);//float変換
			float py = std::stof(py_str);
			float pz = std::stof(pz_str);

			float sx = std::stof(sx_str);
			float sy = std::stof(sy_str);
			float sz = std::stof(sz_str);

			float rx = std::stof(rx_str);
			float ry = std::stof(ry_str);
			float rz = std::stof(rz_str);

			Vector3 pos = Vector3(px, py, pz);
			Vector3 scale = Vector3(sx, sy, sz);
			Vector3 rot = Vector3(rx, ry, rz);

			// 生成関数を呼び出す
			CreateGameObject(name, pos, scale, rot);
		}
		catch (...) {
			// 数値変換に失敗した場合何もしない
			continue;
		}
	}
	// クローズ処理は不要　ifstream型・ofstream型ともにデストラクタにてファイルクローズしてくれるため
}

void Game::SaveObj()
{
	// 全オブジェクトを取得
	std::vector<GameObject*> allObjs = GetAllGameObjects();

	std::ofstream ofs_csv_file("gameObjData.csv");

	// ループしてオブジェクトの情報を書き込む
	for (GameObject* obj : allObjs)
	{
		// セーブ対象フラグが立っているものだけ書き込む
		if (obj->GetSaveObj())
		{
			// 名前, x, y, z
			Vector3 pos = obj->GetPos();
			Vector3 scale = obj->GetScale();
			Vector3 rot = obj->GetRot();
			ofs_csv_file << obj->GetName() << ","
				<< pos.x << ","
				<< pos.y << ","
				<< pos.z << ","
				<< scale.x << ","
				<< scale.y << ","
				<< scale.z << ","
				<< rot.x << ","
				<< rot.y << ","
				<< rot.z << std::endl;

		}
	}
}

void Game::Init()
{
	m_checkPointNum = 0;


	isFirst = true;
	MouseInput::SetUIMode(false);

	AddGameObject<Camera>(0);
	AddGameObject<ShadowCamera>(0);
	AddGameObject<Sky>(l_SKY)->SetScale({ 200.0f,200.0f,200.0f });

	//メッシュフィールド
	//auto meshField = AddGameObject<MeshField>(0);

	//753 873 325 102

	//オブジェクト配置
	auto p = AddGameObject<Player>(l_NOT_TOUMEI);
	p->SetPos({ 0.0f, 0.0f, 0.0f });
	p->CreateZoneUI();

	auto gb = AddGameObject<GravityBall>(l_NOT_TOUMEI);
	gb->SetPos({ 0.0f, 5.0f, -10.0f });
	gb->SetResetPos({ 0.0f, 5.0f, -10.0f });

	gb = AddGameObject<GravityBall>(l_NOT_TOUMEI);
	gb->SetPos({ 400.0f, 96.0f, 400.0f });
	gb->SetResetPos({ 400.0f, 98.0f, 400.0f });
	gb->SetGoalNum(1);

	gb = AddGameObject<GravityBall>(l_NOT_TOUMEI);
	gb->SetPos({ 753.0f, 102.0f, 325.0f });
	gb->SetResetPos({ 753.0f, 102.0f, 325.0f });
	gb->SetGoalNum(2);


	auto sa = AddGameObject<ShootArea>(l_NOT_TOUMEI_BLOOM);
	sa->SetPos({ 0.0f, 6.0f, -40.0f });
	sa->SetScale({ 5.0f, 5.0f, 5.0f });

	auto shootChar2 = AddGameObject<StrTex>(l_GAZOU);
	shootChar2->Init("SHOOT", XMFLOAT4(0.25, 0.05, 0.5, 3.1f), false, Vector3(0, 1, 0));
	shootChar2->SetScale({ 5.0f,5.0f,1.0f });
	shootChar2->SetPos({ 12.5f, 14.5f, -40.0f });
	shootChar2->SetRot({ 0.0f, XM_PI, 0.0f });
	shootChar2->SetRadius(2.0f);
	shootChar2->LightSwitch(true);//ここで実体作ってるけど微妙かも？
	shootChar2->SetLitColor(XMFLOAT3(0, 1, 1));


	sa = AddGameObject<ShootArea>(l_NOT_TOUMEI_BLOOM);
	sa->SetPos({ 480.0f, 110.0f, 400.0f });
	sa->SetScale({ 5.0f, 5.0f, 5.0f });
	sa->SetRot({ 0, XM_PI * 0.5f, 0.0f });

	sa = AddGameObject<ShootArea>(l_NOT_TOUMEI_BLOOM);
	sa->SetPos({ 843.0f, 103.0f, 325.0f });
	sa->SetScale({ 5.0f, 5.0f, 5.0f });
	sa->SetRot({ 0, XM_PI * 0.5f, 0.0f });

	// オブジェクト情報をCSVから読み込み生成
	LoadObj();


	{
	//ディファードライト大量描画サンプル
	
		// ==========================================
		// パターン1: グリッド・ウェーブ (地面の賑やかし)
		// ==========================================
		// 地面 (50x50) を少しはみ出すくらいに配置して広さを演出
		int gridCount = 24;       // 12x12 = 144個
		float spacing = 5.0f;     // 5m間隔
		float offset = (gridCount * spacing) * 0.5f;

		for (int x = 0; x < gridCount; x++)
		{
			for (int z = 0; z < gridCount; z++)
			{
				auto light = AddGameObject<PointLight>(l_LIGHT);

				// 中心を原点に合わせる
				float posX = (x * spacing) - offset + (spacing * 0.5f) - 60;
				float posZ = (z * spacing) - offset + (spacing * 0.5f) - 60;

				// 地面(Y=0)より少し上に配置
				light->SetPos({ posX, 1.5f, posZ });

				// 範囲: 干渉しすぎない程度に
				light->SetScale({ 6.0f, 6.0f, 6.0f });
				light->SetIntensity(1.5f);

				// 色: 市松模様 (シアン & マゼンタ)
				if ((x + z) % 2 == 0)
					light->SetColor({ 0.0f, 1.0f, 1.0f }); // シアン
				else
					light->SetColor({ 1.0f, 0.0f, 0.8f }); // マゼンタ

				// グリッド波の挙動を設定
				// phase を座標に基づいてズラすと、波打っているように見える
				// speed = 2.0f
				light->SetBehavior(PointLight::BehaviorType::GridWave, 2.0f, 0.0f);
			}
		}


		// ==========================================
		// パターン2: スパイラル・タワー (中央のランドマーク)
		// ==========================================
		// 中央から天に向かって伸びる光の螺旋
		int spiralCount = 60;
		float towerHeight = 40.0f;

		for (int i = 0; i < spiralCount; i++)
		{
			auto light = AddGameObject<PointLight>(l_LIGHT);

			float t = (float)i / spiralCount;

			// 初期位置 (Updateで上書きされるが、基準高さとしてYを入れる)
			// Y: 2.0f ～ 42.0f
			light->SetPos({ 0.0f, 2.0f + t * towerHeight, 0.0f });

			light->SetScale({ 8.0f, 8.0f, 8.0f });
			light->SetIntensity(2.0f);

			// 色: 下から上へ虹色グラデーション
			light->SetColor(HsvToRgb(t * 360.0f, 1.0f, 1.0f));

			// 螺旋の挙動を設定
			// phase を i に応じてズラすことで、円状に配置される
			// phase = i * 0.5f (ラジアン単位のズレ)
			light->SetBehavior(PointLight::BehaviorType::Spiral, 1.5f, i * 0.5f);
		}


		// ==========================================
		// パターン3: ハイウェイ (外側のスピード感)
		// ==========================================
		// 地面の左右 (X = ±35) に走る光のライン
		int highwayCount = 50;
		float roadLength = 250.0f; // ループする長さ

		for (int i = 0; i < highwayCount; i++)
		{
			// --- 左車線 (青・奥へ) ---
			{
				auto light = AddGameObject<PointLight>(l_LIGHT);
				// 初期位置のZをバラけさせる
				float startZ = (float)i * (roadLength / highwayCount);

				// 地面より少し外側 (X=-35)
				light->SetPos({ 35.0f, 3.0f, startZ});
				light->SetScale({ 10.0f, 10.0f, 10.0f });
				light->SetIntensity(3.0f);
				light->SetColor({ 0.2f, 0.2f, 1.0f }); // 青

				// ハイウェイ挙動
				light->SetBehavior(PointLight::BehaviorType::Highway, 2.0f);
			}

			// --- 右車線 (赤・手前へ) ---
			// ※ Highwayロジックは一方向のみなので、180度回転させるか、
			//    単純に「逆進する」ロジックをPointLightに追加するのが楽ですが、
			//    ここでは「色違いの同方向」として配置します（十分早そうに見えるので）
			{
				auto light = AddGameObject<PointLight>(l_LIGHT);
				float startZ = (float)i * (roadLength / highwayCount) + (roadLength * 0.5f); // 互い違いにする

				light->SetPos({ 95.0f, 3.0f, startZ});
				light->SetScale({ 10.0f, 10.0f, 10.0f });
				light->SetIntensity(3.0f);
				light->SetColor({ 1.0f, 0.2f, 0.0f }); // オレンジ赤

				light->SetBehavior(PointLight::BehaviorType::Highway, 2.5f); // 少し速度を変える
			}
		}
	}


	// ビル群配置

	const float spacing = 400.0f;

	for (int x = -3; x <= 3; x++)
	{
		for (int z = -3; z <= 3; z++)
		{
			Vector3 pos;
			pos.x = x * spacing;
			pos.z = z * spacing;

			if ((pos.x * pos.x + pos.z * pos.z) < (200.0f * 200.0f))
				continue;

			pos.y = 1.0f;

			//ここだけ大きくしたい
			const float SCALE_MULT = 3.0f;

			Vector3 scale;
			scale.x = (15.0f + ((rand() % 100) / 100.0f)) * SCALE_MULT;
			scale.y = 15.5f * SCALE_MULT;
			scale.z = (15.0f + ((rand() % 100) / 100.0f)) * SCALE_MULT;

			Vector3 rot;
			rot.x = (rand() % 360) * (3.14159f / 180.0f);
			rot.y = (rand() % 360) * (3.14159f / 180.0f);
			rot.z = (rand() % 360) * (3.14159f / 180.0f);

			auto tree = AddGameObject<Building>(l_NOT_TOUMEI_BLOOM);
			tree->SetPos(pos);
			tree->SetScale(scale);
		}
	}
	

	
	// パーティクル
	AddGameObject<RealityFragment>(l_PARTICLE)->Init(20000, "asset\\texture\\sakura.png");
	
	//スコア
	AddGameObject<Score>(l_UI);
	//

	// BOXの設置
	GroundBox* gbox;
	// これベースの地面
	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ 0.0f, -5.0f, 0.0f });
	gbox->SetScale({ 150.0f, 5.0f, 150.0f });

	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ 0.0f, 10.0f, 10.0f });
	gbox->SetScale({ 5.0f, 5.0f, 5.0f });

	gbox;
	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ -10.0f, 5.0f, 10.0f });
	gbox->SetScale({ 5.0f, 5.0f, 5.0f });

	gbox;
	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ -10.0f, 20.0f, 10.0f });
	gbox->SetScale({ 5.0f, 2.0f, 5.0f });
	
	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ 0.0f, 25.0f, 10.0f });
	gbox->SetScale({ 5.0f, 2.0f, 5.0f });

	//kanni
	gbox = AddGameObject<GroundBox>(l_NOT_TOUMEI);
	gbox->SetPos({ 200.0f, 30.0f, 0.0f });
	gbox->SetScale({ 10.0f, 2.0f, 10.0f });

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM)->SetPos({ 270.0f, 80.0f + i, -2.5f + j });
		}
	}

	//オブジェクトとしての雲は最適化出来ていない為一旦無し
	//Cloud* cloud;
	//cloud = AddGameObject<Cloud>(l_CLOUD);
	//cloud->SetPos({ 20.0f, 30.0f, 00.0f });
	//cloud->SetScale({ 40.0f, 20.0f, 30.0f });


	// 3d空間Text

	auto testChar = AddGameObject<StrTex>(l_GAZOU);
	testChar->Init("HELLO", XMFLOAT4(0.25, 0.5, 0.08, 1.1f));
	testChar->LightSwitch(true);//ここで実体作ってるけど微妙かも？
	testChar->SetLitColor(XMFLOAT3(0, 1, 0));
	testChar->SetPos({ 6.0f, 4.0f, 9.0f });

	// スケールを後で確認
	// 回転によって形状を変形させたい
	auto testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("DIRECTX", XMFLOAT4(0.5, 0.1, 0.05, 1.1f),true,Vector3(1,1,0));
	testChar2->LightSwitch(true);//ここで実体作ってるけど微妙かも？
	testChar2->SetLitColor(XMFLOAT3(0, 0, 1));
	testChar2->SetPos({ 6.0f, 4.0f, 9.0f });

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("UNITY", XMFLOAT4(0.25, 0.05, 0.5, 1.1f), true, Vector3(-1, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar2->SetPos({ 6.0f, 4.0f, 9.0f });

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("UNREAL", XMFLOAT4(0.25, 0.05, 0.5, 1.1f), true, Vector3(0, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(1, 1, 1));
	testChar2->SetPos({ 6.0f, 1.0f, 9.0f });
	testChar2->SetRadius(2.0f);
	testChar2->SetLitA(0.2f);

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("GAMEAREA", XMFLOAT4(0.25, 0.05, 0.5, 0.5f), true, Vector3(0, 1, 0));
	testChar2->SetScale({ 7.0f,7.0f,1.0f });
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(1, 1, 1));
	testChar2->SetPos({ 6.0f, 30.0f, 40.0f });
	testChar2->SetRadius(20.0f);
	testChar2->SetLitA(0.2f);


	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("TEXT", XMFLOAT4(0.25, 0.05, 0.5, 0.1f), false, Vector3(0, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar2->SetPos({ 0.0f, 1.0f, 3.0f });
	testChar2->SetRadius(2.0f);

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("TEXT", XMFLOAT4(0.25, 0.05, 0.5, 1.1f), false, Vector3(0, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar2->SetPos({ 0.0f, 2.5f, 3.0f });
	testChar2->SetRadius(2.0f);

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("TEXT", XMFLOAT4(0.25, 0.05, 0.5, 2.1f), false, Vector3(0, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar2->SetPos({ 0.0f, 4.5f, 3.0f });
	testChar2->SetRadius(2.0f);

	testChar2 = AddGameObject<StrTex>(l_GAZOU);
	testChar2->Init("TEXT", XMFLOAT4(0.25, 0.05, 0.5, 3.1f), false, Vector3(0, 1, 0));
	testChar2->LightSwitch(true);
	testChar2->SetLitColor(XMFLOAT3(0, 1, 1));
	testChar2->SetPos({ 0.0f, 6.5f, 3.0f });
	testChar2->SetRadius(2.0f);

	// 簡易的オブジェクト配置

	AddGameObject<Coin>(l_NOT_TOUMEI_BLOOM)->SetPos({ -5.0f, 3.0f, 4.0f });

	for (int i = 0; i < 10; i++)
	{
		AddGameObject<Coin>(l_NOT_TOUMEI_BLOOM)->SetPos({ 25.0f, 3.0f, 10.0f + i * 2 });
	}


	AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM)->SetPos({ -5.0f, 1.0f, 2.0f });
	AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM)->SetPos({ -6.0f, 1.0f, 2.0f });
	AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM)->SetPos({ -7.0f, 1.0f, 2.0f });

	for (int i = 0; i < 10; i++)
	{
		AddGameObject<PointObject>(l_NOT_TOUMEI_BLOOM)->SetPos({ 20.0f, 5.0f, 10.0f + i * 2 });
	}

	// 第一チェックポイント設置
	AddGameObject<CheckPoint>(l_HANTOUMEI_BLOOM)->Init(1);

	// BGM
	m_BGM = new Audio();
	m_BGM->Load("asset\\audio\\Sprinkle_Sparkle.wav");
	m_BGM->Play(true);


}
void Game::Update()
{
	//アップデートの１フレーム目はプレイヤーの位置をリセットする
	//時間ベースで計算しているため１フレーム目は前フレームとの時間の差がとんでもないことになり
	//プレイヤーがすり抜けるため
	//現在は問題ないはずだけどまあ最初のフレームの動作を設定する感じで
	if (isFirst)
	{
		auto p = GetGameObject<Player>();
		p->SetPos({ 0.0f, 2.0f, 0.0f });
		p->SetResetPos({ 0.0f, 2.0f, 0.0f });

		isFirst = false;
		
	}

	Scene::Update();


	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::SetScene<Result>();
	}
}

void Game::Uninit()
{
	//時間を念のため初期設定に戻す
	Time::SetTimeScale(1.0f);

	m_BGM->Uninit();
	delete m_BGM;

	Scene::Uninit();
	MouseInput::SetUIMode(true);
}
