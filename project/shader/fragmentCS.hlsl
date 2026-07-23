#include "common.hlsl"

// C++側で定義したParticle構造体と完全に一致させる
struct Particle
{
    float3 Position;
    float3 Velocity;
    float Life;
    float Age;
    float Size; //サイズを追加
    float Rotation;
    float AngularVelocity;
    uint IsActive;
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
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    Particle p = g_InputParticles[idx];
     //  基本的な力を定義
    float3 baseGravity = float3(0.0, -0.1, 0.0);
    float3 gentleWind = float3(sin(p.Age * 2.0 + idx), 0, cos(p.Age * 1.5 + idx * 0.5)) * 0.2;

    // プレイヤー情報を取得
    float3 playerPos = zone.PlayerPosition.xyz;
    float3 playerVel = zone.PlayerVelocity.xyz;


    if (p.Life <= 0.0f || p.IsActive == 0)
    {
       
    
        float randVal = random(idx + zone.TotalTime);
      // プレイヤーの周囲に再発生
        p.Position = playerPos + float3(
        random(idx * 10.123 + zone.TotalTime) * 40.0 - 20.0, // X用のシード
        random(idx * 20.456 + zone.TotalTime) * 40.0 - 20.0, // Y用のシード
        random(idx * 30.789 + zone.TotalTime) * 40.0 - 20.0 // Z用のシード
    );
        p.Velocity = 0;
        p.Life = 1.0 + randVal * 2.0;
        p.Age = 0.0;
        p.Size = 0.1 + randVal * 0.1;
        p.AngularVelocity = (random(idx * 29u) - 0.5) * 4.0;
        p.Rotation = random(idx * 31u) * 6.28; // 初期角度もランダムに
        p.IsActive = 1;
        
    }
    else
    {
    
    
        p.Age += zone.DeltaTime;
        p.Life -= zone.DeltaTime * 0.5f;
        p.Rotation += p.AngularVelocity * zone.DeltaTime; // 角度を更新

    // --- 物理計算 ---

   
    // プレイヤーの速度に応じて影響範囲を動的に計算
        float playerSpeed = length(playerVel) * 0.01;
        float minRadius = 5.0f;
        float maxRadius = 50.0f;
        float radiusBySpeed = minRadius + (playerSpeed * 2.0f);
        float effectRadius = min(radiusBySpeed, maxRadius); // 上限を超えないように
        float effectRadiusSq = effectRadius * effectRadius;

    // プレイヤーの航跡による力を計算
        float3 slipstreamForce = float3(0, 0, 0);
        float distSq = dot(p.Position - playerPos, p.Position - playerPos);
    
        if (distSq < effectRadiusSq)
        {
            float influence = 1.0 - (distSq / effectRadiusSq);
            slipstreamForce = -playerVel * influence * 10.0f;
            p.AngularVelocity += length(playerVel) * influence * 0.1f;
        }

    // 全ての力を合成し、速度に加える
        p.Velocity += (baseGravity + gentleWind + slipstreamForce) * zone.DeltaTime;
    
    // 空気抵抗で速度を減衰させ、位置を更新
        p.Velocity *= 0.98f;
        p.Position += p.Velocity * zone.DeltaTime;

   
    }
    g_OutputParticles[idx] = p;
}
