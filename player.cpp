#include "main.h"
#include "manager.h"
#include "player.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "GravityZone.h"
#include "circleUI.h"
#include "scene.h"
#include "selectPoint.h"
#include "Time.h"
#include "collision.h"
#include "animationModel.h"
#include "meshField.h"
#include "groundBox.h"
#include "coin.h"
#include "score.h"
#include "audio.h"
#include "result.h"
#include "Building.h"
#include "StrUI.h"
#include "PointObject.h"
#include "imgui.h"


void Player::Init()
{
	//アニメーションやモデル関連のロード
	m_animationModel = new AnimationModel();
	m_animationModel->Load("asset\\model\\test2.fbx");
	m_animationModel->LoadAnimation("asset\\model\\test2_Run.fbx","Run");
	m_animationModel->LoadAnimation("asset\\model\\test2_Idle.fbx","Idle");
	m_animationModel->LoadAnimation("asset\\model\\test2float3.fbx","Float");


	//シェーダー
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");

	//UIの生成（これプレイヤーが生成すべきか分からんけど位置を覚えやすいためここ）
	circleUi = Manager::GetScene()->AddGameObject<CircleUI>(l_UI);
	circleUi->Init(
		ScreenSize::ScreenWidth / 2.0f, ScreenSize::ScreenHeight / 2.0f, 170.0f, 230.0f, 64                               
	);
	//初期化
	m_velocity = { 0.0f,0.0f,0.0f };
	m_gravityVelocity = { 0.0f,0.0f,0.0f };
	m_acc = { 0.0f,0.0f,0.0f };
	m_gravity = 0;

	//アニメーション群
	m_Frame = 0;
	m_AnimationName = "Idle";
	m_AnimationNameNext = "Run";
	m_AnimationBlend = 0.0f;

	//サウンド
	m_SE = new Audio();
	m_SE->Load("asset\\audio\\kira.wav");
	isFirst=true;

	// OBBの情報を設定する
	m_obb.Center = m_position;
	m_obb.Center.y += m_scale.y;
	m_obb.Extents = { m_scale.x * 0.5f, m_scale.y, m_scale.z * 0.5f };

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);

	//FOV変数の初期化
	m_fov = 60.0f;

	m_baseFov = 60.0f;
	m_maxFov = 75.0f;
	m_currentFov = m_baseFov;
	m_fovResetTimer = 0.0f;

	

}
void Player::Uninit()
{
	//終了

	m_animationModel->Uninit();
	delete m_animationModel;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_SE->Uninit();
	delete m_SE;

}

void Player::Update()
{
	if (sceneNo == 0)
	{//これタイトル時の簡易的動作（まあこのままでも）

		m_position = { 0,10,-20 };
		m_AnimationName = "Float";
		m_AnimationNameNext = "Float";
		//アニメーション関連
		m_animationModel->Update(m_AnimationName.c_str(), m_Frame,
			m_AnimationNameNext.c_str(), m_Frame, m_AnimationBlend);//.c_strで変換 frameは別のほうがいい
		m_Frame += 100 * Time::GamePlayTime();//もと１

		m_AnimationBlend += 15.0f * Time::GamePlayTime();//元0.15
		if (m_AnimationBlend > 1.0f)
			m_AnimationBlend = 1.0f;
		return;
	}

	//デバック用処理--------------------------------------
	if (Manager::GetDebug())
	{//managerでBキーに設定
		//この時にはカメラがフリーカメラになる
		return;
	}

	//----------------------------------------------------

	//リセット状態を戻す
	if(m_resetScene)
	m_resetScene = false;

	//UI更新
	if (testUI)
	{
		std::string text = std::to_string(MAX_ZONE_COUNT - m_zoneCount) + "/" + std::to_string(MAX_ZONE_COUNT);

		testUI->SetText(text);
	}

	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();

	Vector3 rotation = camera->GetRot();
	//m_rotation.y = rotation.y;

	if (Input::GetKeyTrigger('R'))
	{
		m_resetScene = true;
	}

	//移動入力
	//空中ではゾーンの生成位置の選択でもよいかも
	Vector3 moveVel = { 0,0,0 };
	if (Input::GetKeyPress('A'))
	{
		moveVel += -camera->GetRight();
	
	}
	if (Input::GetKeyPress('D'))
	{
		moveVel += camera->GetRight();
		
	}
	if (Input::GetKeyPress('W'))
	{
		Vector3 forward = camera->GetForward();
		forward.y = 0.0f;
		forward.Normalize();

		moveVel += forward;

	}
	if (Input::GetKeyPress('S'))
	{
		Vector3 forward = -camera->GetForward();
		forward.y = 0.0f;
		forward.Normalize();

		moveVel += forward;
	
	}


	if (moveVel.Length() > 0.0f) //WASDの移動入力がある場合
	{
		const float runPower = 500.0f;

		moveVel.Normalize(); //方向ベクトルの長さを1にする
		moveVel *= runPower * Time::GamePlayTime();

		m_rotation.y = atan2(moveVel.x, moveVel.z);
	
	}

	else//移動入力無し
	{
		
	}



	//遠隔モード
	int wheel = Input::GetWheel();
	const float AddValue = 0.5f;
	if (wheel > 0)
	{
		GravityZone::AddPZdistance(AddValue);//距離を変化
	}
	else if (wheel < 0)
	{
		GravityZone::AddPZdistance(-AddValue);//距離を
	}
	if (Input::GetKeyTrigger('Q'))
	{
		m_zoneCreatePPos = !m_zoneCreatePPos;//遠隔モード切り替え
		GravityZone::SetPZdistance(10.0f);//距離リセット
	}

	// ゾーン生成
	if (Input::GetKeyTrigger(VK_RBUTTON) && m_zoneCount < MAX_ZONE_COUNT) // 右クリック押下でゾーン開始
	{
		
		GravityZone* zone = Manager::GetScene()->AddGameObject<GravityZone>(l_HANTOUMEI); // レイヤーはゾーン用に
		zone->BeginSetting();

		CircleUI::SetIsDraw(true);
		SelectPoint::SetIsDraw(true);
		Camera::SetIsMove(false);
		//スローに
		Time::SetTimeScale(0.3f);

	}


	// 移動量の計算 ----------------------------


	bool isOnGround = false; // このフレームで地面にいるかどうか

	//ゾーンとの判定
	//ゾーンからのvel計算
	auto zones = Manager::GetScene()->GetGameObjects<GravityZone>();
	bool inZone = false;

	float targetFov = m_baseFov;
	for (auto zone : zones)
	{
		if (!zone->GetSetting())
		{
			if (Collision::CheckCollision_OBB_OBB(m_obb, zone->GetObb()).isColliding)
			{
				// ゾーン内にいる
				inZone = true;
				m_gravity = 0;
				m_gravityVelocity += zone->GetGravityDirection() * (10.0f * Time::GamePlayTime());
				char str[128];
				sprintf_s(str, "zone\n%f", zone->GetGravityDirection().y);
				OutputDebugStringA(str);

				targetFov = m_maxFov;

				// ゾーン中はリセットタイマーを常に満タンにしておく
				m_fovResetTimer = 1.0f; // ゾーンを出てから0.5秒後に戻り始める
			}
		}
	}

	if (!inZone)
	{
		m_gravityVelocity.y -= 10.0f * Time::GamePlayTime();
		// ゾーンに入っていない場合のFOV制御
		if (m_fovResetTimer > 0.0f)
		{
			// まだ「余韻」の時間中。ターゲットは広角のまま維持（または今のFOVをキープ）
			targetFov = m_maxFov;
			m_fovResetTimer -= Time::GamePlayTime();
		}
		else
		{
			// 時間が経ったら元の画角に戻す
			targetFov = m_baseFov;
		}
	}
	
	// FOVのスムーズな補間
	// current を target に近づける
	// Time::GamePlayTime() * Speed で滑らかに変化
	float diff = targetFov - m_currentFov;
	m_currentFov += diff * m_fovChangeSpeed * Time::GamePlayTime();

	// カメラに適用
	Manager::GetScene()->GetGameObject<Camera>()->SetFovDegree(m_currentFov);

	// c. 最終的な移動ベクトルを計算
	m_velocity = moveVel + m_gravityVelocity;
	Vector3 moveVector = m_velocity * Time::GamePlayTime();


	// 衝突判定と応答 ---------------------------
	auto groundBoxes = Manager::GetScene()->GetGameObjects<GroundBox>();
	for (auto box : groundBoxes)
	{
		// a. プレイヤーを移動先に動かしたと仮定してOBBを更新
		Obb nextFramePlayerObb = GetObb();
		nextFramePlayerObb.Center = nextFramePlayerObb.Center + moveVector;

		CollisionResult result = Collision::CheckCollision_OBB_OBB(nextFramePlayerObb, box->GetObb());

		if (result.isColliding)
		{
			// b. 衝突していたら、まず移動ベクトルを押し戻す
			moveVector += result.mtv;

			// c. 押し出しベクトルの向きを調べて、地面か壁かを判断
			Vector3 normal = result.mtv;
			normal.Normalize();

			if (normal.y > 0.7f) // 押し出しがほぼ真上なら「地面」
			{
				isOnGround = true;
				if (m_gravityVelocity.y < 0) {
					m_gravityVelocity.y = 0; // 落下速度をリセット
				}
			}
			else // それ以外は「壁」や「天井」
			{
				float dot = Vector3::Dot(moveVector, normal);
				moveVector -= normal * dot;
			}
		}
	}
	
	//
	// ビル同士が近いと互いのすり抜け防止ですり抜ける
	//
	auto buil = Manager::GetScene()->GetGameObjects<Building>();
	for (auto box : buil)
	{
		// a. プレイヤーを移動先に動かしたと仮定してOBBを更新
		Obb nextFramePlayerObb = GetObb();
		nextFramePlayerObb.Center = nextFramePlayerObb.Center + moveVector;

		CollisionResult result = Collision::CheckCollision_OBB_OBB(nextFramePlayerObb, box->GetObb());

		if (result.isColliding)
		{
			// b. 衝突していたら、まず移動ベクトルを押し戻す
			moveVector += result.mtv;

			// c. 押し出しベクトルの向きを調べて、地面か壁かを判断
			Vector3 normal = result.mtv;
			normal.Normalize();

			if (normal.y > 0.7f) // 押し出しがほぼ真上なら「地面」
			{
				isOnGround = true;
				if (m_gravityVelocity.y < 0) {
					m_gravityVelocity.y = 0; // 落下速度をリセット
				}
			}
			else // それ以外は「壁」や「天井」
			{
				float dot = Vector3::Dot(moveVector, normal);
				moveVector -= normal * dot;
			}
		}
	}



	// 最終的な位置を更新 -------------------------

	if (isFirst)
	{
		isFirst = false;
		return;
	}
	else
	{
		m_position += moveVector;
		m_velocity = moveVel + m_gravityVelocity;
	}

	// 逆入力によるブレーキ処理//地上のみ
	if (isOnGround)
	{
		// 水平方向のベクトルだけを取り出す（落下にはブレーキをかけないため）
		Vector3 inputDir = { moveVel.x, 0.0f, moveVel.z };
		Vector3 inertiaDir = { m_gravityVelocity.x, 0.0f, m_gravityVelocity.z };

		// 入力があり、かつ慣性スピードがある場合のみ判定
		if (inputDir.LengthSq() > 0.001f && inertiaDir.LengthSq() > 0.001f)
		{
			inputDir.Normalize();
			inertiaDir.Normalize();

			// 内積を計算
			// dot > 0 : 同じ方向に進もうとしている（加速）
			// dot < 0 : 逆方向に進もうとしている（ブレーキ）
			float dot = Vector3::Dot(inputDir, inertiaDir);

			// 逆入力（ブレーキ）なら慣性を急激に殺す
			// -0.2f などの閾値で「ある程度逆向きなら」と判定
			if (dot < -0.2f)
			{
				// ブレーキ係数（0.0fに近いほど一瞬で止まる。0.01f ～ 0.1f くらいで調整）
				// powを使うことでフレームレートに依存せず減衰させます
				float brakePower = 0.005f;

				m_gravityVelocity.x *= pow(brakePower, Time::GamePlayTime());
				m_gravityVelocity.z *= pow(brakePower, Time::GamePlayTime());
			}
		}
	}
	if (isOnGround)
	{//地面なら摩擦的なのをつよく？
		m_velocity *= pow(0.58f, Time::GamePlayTime());
		m_gravityVelocity.x *= pow(0.58f, Time::GamePlayTime());
		m_gravityVelocity.z *= pow(0.58f, Time::GamePlayTime());

		//地面にいるならゾーンカウントリセット
		m_zoneCount = 0;
	}
	else
	{
		m_velocity *= pow(0.88f, Time::GamePlayTime());
		m_gravityVelocity.x *= pow(0.88f, Time::GamePlayTime());
		m_gravityVelocity.z *= pow(0.88f, Time::GamePlayTime());
	}


	//落下判定
	if (m_position.y < -50.0f&&!isFirst&& !m_changeScene)
	{
		//
		m_position = m_resetPos;
		m_velocity = { 0.0f,0.0f,0.0f };
		m_gravityVelocity = { 0.0f,0.0f,0.0f };
		m_gravity = 0;
		
	}



	// シーンから全てのCoinを取得
	auto coins = Manager::GetScene()->GetGameObjects<Coin>();

	for (auto coin : coins)
	{
		// プレイヤーとコインの距離を計算
		Vector3 diff = m_position - coin->GetPos();
		float distance = diff.Length();

		// プレイヤーの半径とコインの半径を足す
		float totalRadius = m_radius + coin->GetRadius() + 0.5f;// 0.5fは少し余裕を持たせるための値

		// 距離が半径の合計より小さければ、衝突している
		if (distance < totalRadius && !coin->GetDestroy())
		{
			// コインを取得した処理（コインを消滅させ、スコアを加算）
			coin->SetDestroy();
			auto score = Manager::GetScene()->GetGameObject<Score>();
			score->Add(100);
			m_SE->Play(false, 0.3f);
			break; // 1フレームに1枚だけ取得
		}
	}

	// シーンから全てのobjを取得
	auto point = Manager::GetScene()->GetGameObjects<PointObject>();

	for (auto p : point)
	{
		// プレイヤーとobjの距離を計算
		Vector3 diff = m_position - p->GetPos();
		float distance = diff.Length();

		// プレイヤーの半径とobjの半径を足す
		float totalRadius = m_radius + p->GetRadius() + 0.5f;// 0.5fは少し余裕を持たせるための値

		// 距離が半径の合計より小さければ、衝突している
		if (distance < totalRadius && !p->GetDestroy())
		{
			// objを取得した処理（コインを消滅させ、スコアを加算）
			p->SetDestroy();
			auto score = Manager::GetScene()->GetGameObject<Score>();
			score->Add(10);
			m_SE->Play(false, 0.3f);

			//ゾーンの生成数も回復
			m_zoneCount-=2;
			//0以下回避
			m_zoneCount = std::max(m_zoneCount, 0);

			break; // 1フレームに1枚だけ取得
		}
	}





	//テスト用の地形判定（メッシュフィールド）
	//MeshField* field = Manager::GetScene()->GetGameObject<MeshField>();
	//m_position.y=field->GetHeight(m_position);



	// OBBの情報を更新する
	m_obb.Center = m_position;
	m_obb.Center.y += m_scale.y;
	m_obb.Extents = { m_scale.x * 0.5f, m_scale.y, m_scale.z * 0.5f };

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);



	if (isOnGround)
	{
		//走るアニメーション
		if (moveVel.Length() > 0.0f) //WASDの移動入力がある場合
		{

			//animation
			if (m_AnimationName != "Run")
			{
				m_AnimationName = m_AnimationNameNext;
				m_AnimationNameNext = "Run";
				m_AnimationBlend = 0.0f;
			}

		}
		//待機アニメーション
		else//移動入力無し
		{
			if (m_AnimationName != "Idle")
			{
				m_AnimationName = m_AnimationNameNext;
				m_AnimationNameNext = "Idle";
				m_AnimationBlend = 0.0f;
			}
		}
	}
	//落下アニメーション（ジャンプが無いため意外とこれのみでも？）
	else
	{
		//animation
		if (m_AnimationName != "Float")
		{
			m_AnimationName = m_AnimationNameNext;
			m_AnimationNameNext = "Float";
			m_AnimationBlend = 0.0f;
		}
	}


	//アニメーション関連
#ifdef NDEBUG
	m_animationModel->Update(m_AnimationName.c_str(), m_Frame,
	m_AnimationNameNext.c_str(), m_Frame,m_AnimationBlend);//.c_strで変換 frameは別のほうがいい

	#endif

	m_Frame += 70 * Time::GamePlayTime();//もと１

	m_AnimationBlend += 0.015f * Time::GamePlayTime();//元0.15
	if (m_AnimationBlend > 1.0f)
		m_AnimationBlend = 1.0f;


	

}

void Player::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	//m_modelRenderer->Draw();
	m_animationModel->Draw();
	

}

void Player::DrawShadow()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(Renderer::GetShadowVS() , NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(nullptr, NULL, 0);

	//このへんの計算は変数に代入すべきっぽい
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	//m_modelRenderer->Draw();
	m_animationModel->Draw();
}

void Player::DrawImgui()
{
	if (ImGui::CollapsingHeader(u8"player")) {
		// ヘッダーが開いている時だけ、詳細情報を表示する
		ImGui::Text("Position: %.2f, %.2f", m_position.x, m_position.y);
		ImGui::Text("scale: %.2f, %.2f", m_scale.x, m_scale.y);
		ImGui::Text("ZoneCount: %d", m_zoneCount);
		ImGui::DragFloat("fov", &m_fov, 0.1f);
		// ... 他のパラメータ
	}
}

void Player::CreateZoneUI()
{
	//UI用設定
	if (!testUI)
	{
		testUI = Manager::GetScene()->AddGameObject<StrUI>(l_UI);
		testUI->Init(32);
		testUI->SetText("INIT");
		testUI->SetPos({ ScreenSize::ScreenWidth * 0.75f,ScreenSize::ScreenHeight * 0.85f, 0.0f });
		testUI->SetScale({ ScreenSize::ScreenWidth * 0.1f,ScreenSize::ScreenHeight * 0.15f,1.0f });
	}
}

//memo
// SIMD---
// ひとつの命令で複数のデータを処理
// XMVECTOR
// 
// 使用方法---
// アセンブラ(めんどい)
// 組み込み関数(めんどい)
// 対応ライブラリを使用dx math(ちょっと使いずらい)
// 
// 
// 
//
