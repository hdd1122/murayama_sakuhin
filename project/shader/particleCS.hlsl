#include "common.hlsl"
//
//現在不使用
//

// C++側で定義したParticle構造体と完全に一致させる
struct Particle
{
    float3 Position;
    float3 Velocity;
    float Life;
    float Age;
    float Size; //サイズを追加
    float Rotaion;
    float2 Dummy;
};


// 入力バッファ (前のフレームの状態)
StructuredBuffer<Particle> g_InputParticles : register(t0);

// 出力バッファ (今のフレームの状態)
RWStructuredBuffer<Particle> g_OutputParticles : register(u0);

//疑似ランダム
float random(float seed)
{
    return frac(sin(seed) * 43758.5453);
}

// スレッドグループのサイズ (C++側のDispatchと合わせる)
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint particleIndex = dispatchThreadID.x;

    // 前のフレームのパーティクルデータを読み込む
    Particle p = g_InputParticles[particleIndex];

    // --- ここに物理法則を記述 ---
    
    // 寿命を減らす
    p.Age += zone.DeltaTime;
    p.Life -= zone.DeltaTime * 0.2f; // 寿命の減る速さ

// 1. 横揺れ（風）の力を計算
// AgeとIndexを使い、各パーティクルが異なる周期で揺れるようにする
    float swayFrequency = 2.0f;
    float swayStrength = 0.5f;
    float3 windForce = float3(sin(p.Age * swayFrequency + particleIndex), 0.0f, cos(p.Age * swayFrequency * 0.8f + particleIndex * 0.5f)) * swayStrength;

// 2. 弱い重力と空気抵抗
    float3 gravityForce = float3(0.0f, -0.98f, 0.0f);
    float dragFactor = 0.99f; // 速度が上がりすぎないようにする空気抵抗

// 3. 力を速度に加算
    p.Velocity += (gravityForce + windForce) * zone.DeltaTime;
    p.Velocity *= dragFactor; // 速度を減衰させる

// 4. 速度を使って位置を更新
    p.Position += p.Velocity * zone.DeltaTime;

    // 寿命が尽きたら、リセットする
    if (p.Life <= 0.0f)
    {
        p.Position = float3((particleIndex % 100) - 50.0f, 20.0f, (particleIndex / 100) - 50.0f); // 例: 上から再発生
        float randomAngle = random(particleIndex + zone.TotalTime) * 2.0 * 3.14159;
        p.Velocity = float3(cos(randomAngle), 0.0, sin(randomAngle)) * 0.2f;
        p.Life = 1.0f + (frac(particleIndex * 0.12345) * 4.0); // 寿命をリセット
        p.Age = 0.0f;
        float randomValue = random(particleIndex);
        p.Size = randomValue * 0.2f + 0.2f;
    }
    
    // --- 記述ここまで ---

    // 計算結果を新しいバッファに書き込む
    g_OutputParticles[particleIndex] = p;
}
