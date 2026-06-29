#include "main.h"
#include "manager.h"
#include "GravityBall.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "GravityZone.h"
#include "scene.h"
#include "Time.h"
#include "collision.h"
#include "groundBox.h"
#include "score.h"
#include "audio.h"
#include "result.h"
#include "Building.h"
#include "imgui.h"
#include "ShootArea.h"
#include "coin.h"

//プレイヤーコピーで簡易的に実装

void GravityBall::Init()
{
	m_modelRenderer = new ModelRenderer();
	m_modelRenderer->Load("asset\\gravityBall02.obj");


	//シェーダー
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");

	//初期化
	m_velocity = { 0.0f,0.0f,0.0f };
	m_gravityVelocity = { 0.0f,0.0f,0.0f };
	m_acc = { 0.0f,0.0f,0.0f };
	m_gravity = 0;



	//サウンド
	m_SE = new Audio();
	m_SE->Load("asset\\audio\\collision.wav");
	isFirst=true;

	// OBBの情報を設定する
	m_obb.Center = m_position;
	m_obb.Extents = { m_scale.x * 0.5f, m_scale.y, m_scale.z * 0.5f };

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);


	

}
void GravityBall::Uninit()
{
	//終了
	delete m_modelRenderer;

	m_PixelShader->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_SE->Uninit();
	delete m_SE;

}

void GravityBall::Update()
{
	if (m_destroy)
		return;

	//デバック用処理--------------------------------------
	if (Manager::GetDebug())
	{//managerでBキーに設定
		//この時にはカメラがフリーカメラになる
		return;
	}

	//----------------------------------------------------

	//
	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();

	// 移動量の計算 ----------------------------


	bool isOnGround = false; // このフレームで地面にいるかどうか

	//ゾーンとの判定
	//ゾーンからのvel計算
	auto zones = Manager::GetScene()->GetGameObjects<GravityZone>();
	bool inZone = false;

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

			
			}
		}
	}

	if (!inZone)
	{
		
		m_gravityVelocity.y -= 10.0f * Time::GamePlayTime();
	}
	
	

	// 最終的な移動ベクトルを計算
	m_velocity = m_gravityVelocity;
	Vector3 moveVector = m_velocity * Time::GamePlayTime();


	// 衝突判定と応答 ---------------------------
	auto groundBoxes = Manager::GetScene()->GetGameObjects<GroundBox>();
	for (auto box : groundBoxes)
	{
		Obb nextFramePlayerObb = GetObb();
		nextFramePlayerObb.Center = nextFramePlayerObb.Center + moveVector;

		CollisionResult result = Collision::CheckCollision_OBB_OBB(nextFramePlayerObb, box->GetObb());

		if (result.isColliding)
		{
			// 位置のめり込みを解消
			moveVector += result.mtv;

			Vector3 normal = result.mtv;
			normal.Normalize();

			// 速度の「反射（バウンド）」計算
			float dotVel = Vector3::Dot(m_gravityVelocity, normal);

			// ボールが壁に向かっている（内積がマイナス）時だけ反射させる（壁の中での連続反射バグ防止）
			if (dotVel < 0.0f)
			{
				// 反射ベクトル公式
				m_gravityVelocity = m_gravityVelocity - (normal * (2.0f * dotVel));

				// 反発係数1.0なら永遠に同じ高さまで跳ねる
				float restitution = 0.7f;
				m_gravityVelocity *= restitution;

				float impactSpeed = -dotVel;

				
				const float SOUND_THRESHOLD = 0.5f;

				if (impactSpeed > SOUND_THRESHOLD)
				{
					//音量の計算
					
					const float MAX_IMPACT = 20.0f;
					float calcVolume = (impactSpeed / MAX_IMPACT) * 0.1f;

					//音量が最大値（0.3f）を超えないようにクランプする
					if (calcVolume > 0.3f) calcVolume = 0.3f;

					//ここ
					m_SE->Play(false, calcVolume);
				}
			}

			// 地面判定
			if (normal.y > 0.7f)
			{
				isOnGround = true;
			}
		}
	}
	
	//
	// ビル同士が近いと互いのすり抜け防止ですり抜ける
	//
	auto buil = Manager::GetScene()->GetGameObjects<Building>();
	for (auto box : buil)
	{
		Obb nextFramePlayerObb = GetObb();
		nextFramePlayerObb.Center = nextFramePlayerObb.Center + moveVector;

		CollisionResult result = Collision::CheckCollision_OBB_OBB(nextFramePlayerObb, box->GetObb());

		if (result.isColliding)
		{
			// 位置のめり込みを解消
			moveVector += result.mtv;

			Vector3 normal = result.mtv;
			normal.Normalize();

			// 速度の「反射（バウンド）」計算
			float dotVel = Vector3::Dot(m_gravityVelocity, normal);

			// ボールが壁に向かっている（内積がマイナス）時だけ反射させる（壁の中での連続反射バグ防止）
			if (dotVel < 0.0f)
			{
				// 反射ベクトル公式
				m_gravityVelocity = m_gravityVelocity - (normal * (2.0f * dotVel));

				// 反発係数1.0なら永遠に同じ高さまで跳ねる
				float restitution = 0.7f;
				m_gravityVelocity *= restitution;
			}

			// 地面判定
			if (normal.y > 0.7f)
			{
				isOnGround = true;
			}
		}
	}

	if (!isFirst)
	{
		//ゴール
		auto shootArea = Manager::GetScene()->GetGameObjects<ShootArea>();
		for (auto ball : shootArea)
		{
			// プレイヤーとコインの距離を計算
			Vector3 diff = m_position - ball->GetPos();
			float distance = diff.Length();

			// プレイヤーの半径とコインの半径を足す
			float totalRadius = m_radius + ball->GetRadius() + 0.5f;// 0.5fは少し余裕を持たせるための値

			// 距離が半径の合計より小さければ、衝突している
			if (distance < totalRadius && !ball->GetDestroy())
			{
				SetDestroy();
				ball->SetDestroy();
				if (m_goalNum == 1)//ここの値によってオブジェクト生成するかとか決める。
				{//理想は関数化だったり生成ごとに情報を管理とかすべきだけど、3か所でしか利用しないつもりだから直

					GroundBox* gbox;
					gbox = Manager::GetScene()->AddGameObject<GroundBox>(l_NOT_TOUMEI);
					gbox->SetPos({ 585.0f, 100.0f, 400.0f });
					gbox->SetScale({ 100.0f, 2.0f, 10.0f });
					for (int i = 0; i < 10; i++)
					{
						Manager::GetScene()->AddGameObject<Coin>(l_NOT_TOUMEI_BLOOM)->SetPos({ 500.0f + 15 * i, 104.0f, 400.0f });
					}

				}
				else if (m_goalNum == 2)
				{
					GroundBox* gbox;
					gbox = Manager::GetScene()->AddGameObject<GroundBox>(l_NOT_TOUMEI);
					gbox->SetPos({ 960.0f, 102.0f, 224.0f });
					gbox->SetScale({ 37.0f, 7.0f, 31.5f });
				
					gbox = Manager::GetScene()->AddGameObject<GroundBox>(l_NOT_TOUMEI);
					gbox->SetPos({ 1075.0f, 130.0f, 100.0f });
					gbox->SetScale({ 20.0f, 5.0f, 22.0f });

				}
				else
				{
					auto coin = Manager::GetScene()->AddGameObject<Coin>(l_NOT_TOUMEI_BLOOM);
					coin->SetPos(ball->GetPos());
				}
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
		m_velocity = m_gravityVelocity;
	}

	
	if (isOnGround)
	{//地面なら摩擦的なのをつよく？
		m_velocity *= pow(0.58f, Time::GamePlayTime());
		m_gravityVelocity.x *= pow(0.58f, Time::GamePlayTime());
		m_gravityVelocity.z *= pow(0.58f, Time::GamePlayTime());

		
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



	// OBBの情報を更新する
	m_obb.Center = m_position;
	//m_obb.Center.y += m_scale.y;
	m_obb.Extents = { m_scale.x * 0.5f, m_scale.y, m_scale.z * 0.5f };

	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMStoreFloat4x4(&m_obb.Rotation, rotMatrix);
	

}

void GravityBall::Draw()
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

	m_modelRenderer->Draw();
	
	
}

void GravityBall::DrawShadow()
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

	m_modelRenderer->Draw();
}

void GravityBall::DrawImgui()
{
	// IDスタックに m_No を積む（これで以後の名前が被ってもOKになる）
	ImGui::PushID(m_No);

	// ヘッダーのラベルだけは番号を表示したいので sprintf が必要
	char headerName[32];
	sprintf(headerName, u8"ball %d", m_No);

	if (ImGui::CollapsingHeader(headerName))
	{

		ImGui::DragFloat3("Position", &m_position.x, 0.1f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);

	}

	// 最後に必ず PopID する
	ImGui::PopID();
}



//
