#include "common.hlsl"
#include "raySetting.hlsl"

//CSM用SDFレイマーチ　現在使用しておらず
SamplerState g_sampler : register(s0);



// 法線を計算する
float3 calcNormal(float3 p)
{ //ずらすしてあーだこーだで傾き的なのが
    const float2 e = float2(0.001, 0.0);
    return normalize(float3(
        map_scene(p + e.xyy) - map_scene(p - e.xyy),
        map_scene(p + e.yxy) - map_scene(p - e.yxy),
        map_scene(p + e.yyx) - map_scene(p - e.yyx)
    ));
}



void main(float4 pos : SV_POSITION,
    float2 uv : TEXCOORD0,
    out float out_depth : SV_Depth)
{

    // NDC（クリップ空間）に変換
    float2 screenNDC = uv * 2.0 - 1.0;
    screenNDC.y = -screenNDC.y;

    // 近クリップ面と遠クリップ面のワールド座標を逆変換で求める
    float4 nearClip = float4(screenNDC, 0.0, 1.0); // z = 0 → DirectX の近クリップ（0〜1）を想定
    float4 farClip = float4(screenNDC, 1.0, 1.0); // z = 1 → far

    // r_inverseViewProjection は C++ 側で inverse(matLightVP) を渡す想定
    float4 nearWorld4 = mul(nearClip, r_inverseViewProjection);
    float4 farWorld4 = mul(farClip, r_inverseViewProjection);

    // ホモジニアス除算
    nearWorld4 /= nearWorld4.w;
    farWorld4 /= farWorld4.w;

    float3 rayOrigin = nearWorld4.xyz; // レイの始点
    float3 rayDir = normalize(farWorld4.xyz - nearWorld4.xyz); // 正しいピクセル毎の方向

    // --- ここからレイマーチング（rayOrigin と rayDir を使う） ---
    float t = 0.0;
    bool hit = false;
    float3 p = rayOrigin;

    int maxSteps = 300;
    float maxDist = r_dummy*3;

    float floatHitEps = 0.0015; // ヒット判定（影用は少し大きめが安定）
    float floatMinStep = 0.0005; // 最小ステップ（必ず hitEps より小さく）
    float stepScale = 0.8; // 0.7〜0.9 が良い

    for (int i = 0; i < maxSteps; ++i)
    {
        float d = map_scene(p);

    // まずヒット判定
        if (d < floatHitEps)
        {
            hit = true;
            break;
        }

    // 最小ステップで clamp（ヒット判定後）
        d = max(d, floatMinStep);

    // ステップ量は少し抑える（過進行防止）
        float stepSize = d * stepScale;

        p += rayDir * stepSize;
        t += stepSize;

        if (t > maxDist)
            break;
    }


    if (hit)
    {
        // 世界座標 p を light の VP に投影して深度を求める
        float4 clipPos = mul(float4(p, 1.0), mul(View, Projection)); // View/Projection がライト行列にセットされている前提
        float ndcDepth = clipPos.z / clipPos.w; // DirectX の場合通常 0..1 だが、実装によって -1..1 のこともある
        // 安全のため 0..1 にマップ（DirectX の standard: 0..1）
        out_depth = ndcDepth * 0.5 + 0.5;
    }
    else
    {
        out_depth = 1.0;
    }


}
