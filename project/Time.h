#pragma once

class Time
{
private:
    static float m_deltaTime;
    static float m_totalTime;
    static float m_timeScale;

public:
    static void Init();
    static void Update();

    static float DeltaTime();     // 1フレームの秒数
    static float TotalTime();     // 起動してからの累計秒数
    static float GamePlayTime() { 
        float returnTime = m_deltaTime * m_timeScale;
        if (returnTime > 0.1f)
            returnTime = 0.1;
        return returnTime;
    };
    static void SetTimeScale(float timeScale) { m_timeScale = timeScale; }


};
