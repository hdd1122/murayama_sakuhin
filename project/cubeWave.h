#pragma once
#include "gameobject.h"

class CubeWave : public GameObject
{
private:
    // 20x20のグリッド状に箱を並べる
    static const int GRID_SIZE = 20;
    class Cube* m_Cubes[GRID_SIZE][GRID_SIZE]; // 箱の配列

public:
    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;
};
