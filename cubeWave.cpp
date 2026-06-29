#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "cubewave.h"
#include "cube.h"
#include "time.h"

//タイトル画面用BOX群

void CubeWave::Init()
{
    float interval = 1.5f; // 箱と箱の間隔

    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int z = 0; z < GRID_SIZE; z++)
        {
            // 箱を生成
            m_Cubes[x][z] = new Cube();
            m_Cubes[x][z]->Init();

            // 基本位置を設定（高さYはUpdateで変えるので0）
            float posX = (x - GRID_SIZE / 2.0f) * interval;
            float posZ = (z - GRID_SIZE / 2.0f) * interval;

            m_Cubes[x][z]->SetPos(Vector3(posX, 0.0f, posZ));
            m_Cubes[x][z]->SetScale(Vector3(1.0f, 4.0f, 1.0f)); // 縦長の棒にすると波っぽさが増す
        }
    }
}

void CubeWave::Uninit()
{
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int z = 0; z < GRID_SIZE; z++)
        {
            if (m_Cubes[x][z])
            {
                m_Cubes[x][z]->Uninit();
                delete m_Cubes[x][z];
                m_Cubes[x][z] = nullptr;
            }
        }
    }
}

void CubeWave::Update()
{
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int z = 0; z < GRID_SIZE; z++)
        {
            // 中心からの距離を計算
            float dx = (float)(x - GRID_SIZE / 2);
            float dz = (float)(z - GRID_SIZE / 2);
            float length = sqrtf(dx * dx + dz * dz);

            // Y座標を計算
            float waveHeight = 2.0f * sinf(Time::TotalTime() * -2.0f + length * 0.5f);

            // 現在のX, Z座標を維持しつつ、Yだけ書き換える
            Vector3 currentPos = m_Cubes[x][z]->GetPos();
            currentPos.y = waveHeight;
            m_Cubes[x][z]->SetPos(currentPos);

            // おまけ
            m_Cubes[x][z]->SetRot(Vector3(0.0f, waveHeight * 0.2f, 0.0f));

            float r = (sinf(Time::TotalTime() * 2.0f + length * 0.5f) + 1.0f) * 0.5f;           // ベース
            float g = (sinf(Time::TotalTime() * 2.0f + length * 0.5f + 2.09f) + 1.0f) * 0.5f;   // 120度ずらす
            float b = (sinf(Time::TotalTime() * 2.0f + length * 0.5f + 4.18f) + 1.0f) * 0.5f;   // 240度ずらす

            m_Cubes[x][z]->SetColor(Vector3(r, g, b));
        }
    }


    // 各キューブのUpdateも呼ぶ
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int z = 0; z < GRID_SIZE; z++)
        {
            m_Cubes[x][z]->Update();
        }
    }
}

void CubeWave::Draw()
{
    // 各キューブを描画
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int z = 0; z < GRID_SIZE; z++)
        {
            m_Cubes[x][z]->Draw();
        }
    }
}
