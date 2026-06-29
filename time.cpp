#include "Time.h"
#include <Windows.h>

//時間関連
static LARGE_INTEGER s_frequency;
static LARGE_INTEGER s_prevCounter;

float Time::m_deltaTime = 0.0f;
float Time::m_totalTime = 0.0f;
float Time::m_timeScale = 1.0f;

void Time::Init()
{
    QueryPerformanceFrequency(&s_frequency);
    QueryPerformanceCounter(&s_prevCounter);
    m_deltaTime = 0.0f;
    m_totalTime = 0.0f;
    m_timeScale = 1.0f;
}

void Time::Update()
{
    LARGE_INTEGER currentCounter;
    QueryPerformanceCounter(&currentCounter);

    m_deltaTime = static_cast<float>(currentCounter.QuadPart - s_prevCounter.QuadPart) / s_frequency.QuadPart;
    m_totalTime += m_deltaTime;
    s_prevCounter = currentCounter;
}

float Time::DeltaTime()
{
    return m_deltaTime;
}

float Time::TotalTime()
{
    return m_totalTime;
}
