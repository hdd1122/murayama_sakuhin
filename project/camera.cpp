#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "camera.h"
#include "player.h"
#include "input.h"
#include "scene.h"
#include "imgui.h"
#include "game.h"

#include <algorithm> 

//メインカメラ

bool Camera::m_isMove = true;

void Camera::Init()
{
	
	m_position = { 0.0f,1.0f,-5.0f };
	m_isMove = true;
	
}
void Camera::Uninit()
{


}

void Camera::Update()
{
	//アニメーション関連
#ifdef _DEBUG
	if (Input::GetKeyTrigger('N'))//簡易カメラ固定
	{
		m_isMove = !m_isMove;
	}
#endif

	if (m_isMove)//マウスが動かせるか？
	{
		// マウスの移動量で縦横の回転角度を更新
		  
		m_rotation.y += MouseInput::GetDeltaX() * m_rotSpeed;
		m_rotation.x += MouseInput::GetDeltaY() * m_rotSpeed;

		// 縦回転が行き過ぎないように制限をかける
		float limit = XM_PIDIV2 - 0.01f; // XM_PIDIV2は90度
		if (m_rotation.x > limit) {
			m_rotation.x = limit;
		}
		if (m_rotation.x < -limit) {
			m_rotation.x = -limit;
		}

	}
	if (!Manager::GetDebug())
	{
		Player* player = Manager::GetScene()->GetGameObject<Player>();
		if (player != nullptr)
		{
			m_Target = player->GetPos() + Vector3(0.0f, 2.0f, 0.0f);

			// 横回転と縦回転の両方を考慮した位置を計算
			float distance = 4.0f;
			m_position = m_Target +
				Vector3(
					-sinf(m_rotation.y) * cosf(m_rotation.x) * distance,
					sinf(m_rotation.x) * distance,
					-cosf(m_rotation.y) * cosf(m_rotation.x) * distance
				);
		}
	}
	else
	{//デバッグモードはプレイヤー関係なくここでカメラ移動
		Vector3 moveVel = {0,0,0};
		if (Input::GetKeyPress('A'))
		{
			moveVel += -GetRight();

		}
		if (Input::GetKeyPress('D'))
		{
			moveVel += GetRight();

		}
		if (Input::GetKeyPress('W'))
		{
			Vector3 forward = GetForward();
			forward.y = 0.0f;
			forward.Normalize();

			moveVel += forward;

		}
		if (Input::GetKeyPress('S'))
		{
			Vector3 forward = -GetForward();
			forward.y = 0.0f;
			forward.Normalize();

			moveVel += forward;

		}

		if (Input::GetKeyPress('Q'))
		{
			moveVel.y += 1;
		}

		if (Input::GetKeyPress('E'))
		{
			moveVel.y += -1;
		}

		if (moveVel.Length() > 0.1f)
		{//入力あり
			moveVel.Normalize();
			
			m_position += moveVel * m_moveSpeed;
			m_Target += moveVel * m_moveSpeed;
		}

		float distance = 4.0f;
		m_position = m_Target +
			Vector3(
				-sinf(m_rotation.y) * cosf(m_rotation.x) * distance,
				sinf(m_rotation.x) * distance,
				-cosf(m_rotation.y) * cosf(m_rotation.x) * distance
			);
	}

	//視錐台を最新の状態に更新
	m_Frustum.Update(GetViewMatrix(), GetProjectionMatrix());

}

void Camera::Draw()
{


	
}
void Camera::SetMatrix()
{
	// プロジェクション行列を作成・セット
	m_projection = XMMatrixPerspectiveFovLH(
		m_fov,
		(float)ScreenSize::ScreenWidth / ScreenSize::ScreenHeight,
		m_nearClip,
		m_farClip
	);
	Renderer::SetProjectionMatrix(m_projection);

	
	//Update()で計算した m_position と m_Target を使ってビュー行列を生成する
	XMFLOAT3 up{ 0.0f,1.0f ,0.0f };
	m_view = XMMatrixLookAtLH(
		XMLoadFloat3((XMFLOAT3*)&m_position),
		XMLoadFloat3((XMFLOAT3*)&m_Target),
		XMLoadFloat3(&up)
	);

	// 行列をセット
	Renderer::SetViewMatrix(m_view);
}

void Camera::DrawImgui()
{
	if (ImGui::CollapsingHeader(u8"camera")) {
		// ヘッダーが開いている時だけ、詳細情報を表示する

			// ボタン処理
		if (ImGui::Button("PlayerTeleport_Cam"))
		{
			Manager::GetScene()->GetGameObject<Player>()->SetPos(m_position);
		}

		ImGui::DragFloat("camSpeed", &m_moveSpeed, 0.1f);
		ImGui::DragFloat("camRotSpeed", &m_rotSpeed, 0.005f);
		ImGui::Text("Position: %.2f, %.2f, %.2f", m_position.x, m_position.y, m_position.z);


		Vector3 camDir = GetForward(); // 視線ベクトル（前方ベクトル）
	
		// カメラの「5.0f」だけ手前に出す
		// そのままだとカメラ内部に生成されて見えないことがあるため
		Vector3 spawnPos = m_position + (camDir * 5.0f);

		ImGui::Text("CreatePosition: %.2f, %.2f, %.2f", spawnPos.x, spawnPos.y, spawnPos.z);

		// ボタン処理
		if (ImGui::Button("Save Level Data"))
		{
			Manager::GetScene()->SaveObj();       // セーブ実行
		}
	
		// 生成したいオブジェクトの名前リスト
		static const char* objNames[] = { "PointObject", "Coin","GroundBox"};
		static int currentItem = 0; // 選択中のインデックス

		// コンボボックスで種類を選ぶ
		// "Object Type" はラベル、&currentItem は選択番号、objNames はリスト、IM_ARRAYSIZE は要素数
		ImGui::Combo("Object Type", &currentItem, objNames, IM_ARRAYSIZE(objNames));

		// 生成ボタン
		if (ImGui::Button("Spawn at Camera View"))
		{
			Vector3 baseScale = { 1.0f,1.0f,1.0f };
			Vector3 baseRot = { 0.0f,0.0f,0.0f };
			// 選択された名前と計算した座標で生成
			Manager::GetScene()->CreateGameObject(objNames[currentItem], spawnPos, baseScale,baseRot);
		}

	}
}
