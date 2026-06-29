#pragma once
#include "gameObject.h"

class Zone : public GameObject
{
protected:
    Vector3 m_size{ 1,1,1 };
    Vector3 m_direction{ 0, -1, 0 }; // 表示のための方向
public:
    virtual void Init() override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual void Uninit() override;

    void SetSize(Vector3 size) { m_size = size; }
    void SetDirection(Vector3 dir) { m_direction = dir; }
};
