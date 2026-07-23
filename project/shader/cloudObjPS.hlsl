#include "common.hlsl"

//オブジェクトとして配置する雲
//現在は未使用
//

// C++で作った3Dノイズ
Texture3D<float> g_volumeTex : register(t0);
SamplerState g_sampler : register(s0); // Wrapモード

// G-Buffer (他オブジェクトとの前後関係判定用)
Texture2D g_depthTexture : register(t1);

// 定数バッファ
cbuffer CloudBuffer : register(b12)
{
    float4 CloudColor; // 雲の色
    float4 BoxCenter; // オブジェクトの中心座標 (World)
    float4 BoxSize; // オブジェクトのサイズ (Scale)
    float DensityScale; // 密度の濃さ
    float3 NoiseOffset;
};


// ==========================================
// 交差判定
// ==========================================
bool IntersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax, out float tNear, out float tFar)
{
    // abs が小さすぎる場合に極小値にする
    float3 safeDir = rayDir;
    if (abs(safeDir.x) < 1e-6)
        safeDir.x = sign(safeDir.x) * 1e-6;
    if (abs(safeDir.y) < 1e-6)
        safeDir.y = sign(safeDir.y) * 1e-6;
    if (abs(safeDir.z) < 1e-6)
        safeDir.z = sign(safeDir.z) * 1e-6;

    float3 invDir = 1.0 / safeDir;
    
    float3 t1 = (boxMin - rayOrigin) * invDir;
    float3 t2 = (boxMax - rayOrigin) * invDir;
    
    float3 tMin = min(t1, t2);
    float3 tMax = max(t1, t2);
    
    tNear = max(max(tMin.x, tMin.y), tMin.z);
    tFar = min(min(tMax.x, tMax.y), tMax.z);
    
    return tNear <= tFar && tFar > 0.0;
}

// ==========================================
// 密度関数
// ==========================================
float GetCloudDensity(float3 worldPos)
{
    // ローカル座標 (-0.5 ～ +0.5)
    float3 localPos = (worldPos - BoxCenter.xyz) / BoxSize.xyz;

    if (abs(localPos.x) > 0.55 || abs(localPos.y) > 0.55 || abs(localPos.z) > 0.55)
        return 0.0;

    // 箱の境界フェード (そのまま)
    float3 distFromEdge = 0.5 - abs(localPos);
    float borderFade = smoothstep(0.0, 0.1, min(min(distFromEdge.x, distFromEdge.y), distFromEdge.z));

    // 単純な球体だとみんな同じ形になるので、
    // 大きな低周波ノイズを使って、座標自体をグニャッと曲げる
    
    // オフセットを加算して、雲ごとに違う場所のノイズを読む
    float3 warpPos = localPos + NoiseOffset * 0.2;
    
    // 低周波ノイズを取得 (大きなうねり)
    float distortion = g_volumeTex.SampleLevel(g_sampler, warpPos, 0).r;
    
    // 中心からの距離を計算する際、ノイズを足して「いびつな距離」にする
    // (distortion - 0.5) * 0.3 で、表面をボコボコさせる
    float dist = length(localPos) + (distortion - 0.5) * 0.3;

    // 歪んだ距離を使って基本形状を作る (球体ベースだがボコボコになる)
    float baseShape = 1.0 - smoothstep(0.2, 0.5, dist); // 0.2〜0.5に範囲を広げて柔らかく

    // 詳細ノイズ (FBM)
    float noise = 0.0;
    // ここでも NoiseOffset を足して、模様をランダムにする
    float3 uvw = localPos * 2.5 + float3(r_time * 0.1, 0, 0) + NoiseOffset;
    
    noise += g_volumeTex.SampleLevel(g_sampler, uvw, 0).r * 0.5;
    noise += g_volumeTex.SampleLevel(g_sampler, uvw * 2.0, 0).r * 0.25;
    noise += g_volumeTex.SampleLevel(g_sampler, uvw * 4.0, 0).r * 0.125;

    // 合成 (ノイズでの削り方を少し強める)
    float finalDensity = baseShape - noise * 1.0;

    return saturate(finalDensity * DensityScale) * borderFade;
}



float4 main(PS_IN input) : SV_Target
{
    float3 camPos = zone.CameraPosition.xyz;
    float3 worldPos = input.WorldPos.xyz;
    float3 rayDir = normalize(worldPos - camPos);

    // 箱の定義
    float3 boxMin = BoxCenter.xyz - BoxSize.xyz * 0.5;
    float3 boxMax = BoxCenter.xyz + BoxSize.xyz * 0.5;

    // レイと箱の交差判定
    float tNear, tFar;
    if (!IntersectAABB(camPos, rayDir, boxMin, boxMax, tNear, tFar))
    {
        discard;
    }

    float pixelDist = distance(camPos, worldPos);
    tFar = min(tFar, pixelDist);
    // レイマーチング区間
    float tStart = max(0.0, tNear);
    float tEnd = tFar;
    float rayLen = tEnd - tStart;
    
    if (rayLen <= 0.0)
        discard;

    // 画面サイズをテクスチャから動的に取得
    uint width, height;
    g_depthTexture.GetDimensions(width, height);
    float2 screenUV = input.Position.xy / float2(width, height);
    
    // 深度の読み取りとリニア化
    float depthVal = g_depthTexture.SampleLevel(g_sampler, screenUV, 0).r;
    
    float zNear = 0.1;
    float zFar = 1000.0; // カメラクラスのFar設定
    float sceneDepth = 0.0;

    // 背景(1.0)の場合は無限遠扱いにする
    if (depthVal >= 0.9999)
    {
        sceneDepth = 10000.0; // 雲より確実に遠い値
    }
    else
    {
        // DirectXの深度バッファ [0, 1] を リニア深度 [Near, Far] に変換する公式
        sceneDepth = (zNear * zFar) / (zFar - depthVal * (zFar - zNear));
    }

    // レイマーチング設定
    int steps = 30;
    float stepSize = rayLen / float(steps);
    float3 currentPos = camPos + rayDir * tStart;
    float totalDensity = 0.0;

    for (int i = 0; i < steps; i++)
    {
        // 現在位置までの距離
        float distFromCam = distance(camPos, currentPos);

        // ソフトパーティクル処理
       
        // 壁までの残り距離
        float depthDiff = sceneDepth - distFromCam;
        
        // 壁の裏側に行ったらこれ以上描画しない
        if (depthDiff < 0.0)
            break;

        // 密度取得
        float d = GetCloudDensity(currentPos);

        if (d > 0.001)
        {
            // ソフトフェード: 壁に近いほど透明にする (2.0m手前から消え始める)
            float softFade = saturate(depthDiff / 2.0);
            d *= softFade;

            // アルファ合成
            totalDensity += (1.0 - totalDensity) * d * stepSize * 0.5;
        }

        if (totalDensity >= 1.0)
            break;

        currentPos += rayDir * stepSize;
    }

    if (totalDensity <= 0.0)
        discard;

    float3 finalColor = CloudColor.rgb * lerp(1.0, 0.5, totalDensity);
    return float4(finalColor, totalDensity);
}
