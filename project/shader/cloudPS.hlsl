#include "common.hlsl"

//めも
// 3Dノイズを用いたボリュームレンダリングによる雲の描画。
// Weighted Blended OITに対応し、半透明合成を行う。
// 負荷軽減のため、LOD制御と早期終了を実装。

Texture3D<float> g_volumeTex : register(t0);
SamplerState g_sampler : register(s0);
Texture2D g_worldPosTexture : register(t1);

cbuffer CloudParams : register(b13)
{
    matrix InvViewProj;
    float4 CloudColor;
    float4 EyePos;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct OIT_PS_OUTPUT
{
    float4 Accumulation : SV_Target0;
    float4 Revealage : SV_Target1;
};

// 簡易乱数生成
float Random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

// UVミラーリング
float3 GetMirroredUV(float3 p)
{
    return 1.0 - abs(1.0 - 2.0 * frac(p * 0.5));
}


// 雲の範囲
static const float CLOUD_MIN_Y = -60.0;
static const float CLOUD_MAX_Y = 10.0;

// ==========================================
// 密度関数：LOD対応
// ==========================================
// lodMode: trueなら軽量モード（詳細ノイズを無視）
float GetCloudDensity(float3 pos, bool lodMode)
{
    // 範囲外チェック
    if (pos.y < CLOUD_MIN_Y || pos.y > CLOUD_MAX_Y)
        return 0.0;

    // --- 高さによる早期リターン ---
    float heightFraction = (CLOUD_MAX_Y - pos.y) / (CLOUD_MAX_Y - CLOUD_MIN_Y);
    float heightSignal = sin(heightFraction * 3.14159);
    heightSignal = pow(heightSignal, 0.5);
    
    if (heightSignal < 0.1)
        return 0.0;


    // --- Baseノイズだけ読む ---
    float3 noiseCoord = pos * float3(0.003, 0.004, 0.003);
    noiseCoord += float3(r_time * 0.015, r_time * 0.005, r_time * 0.01);
    
    float baseNoise = g_volumeTex.SampleLevel(g_sampler, GetMirroredUV(noiseCoord), 0).r;

    // 雲の基本形状を作成
    float estimatedCloud = heightSignal + (baseNoise * 0.5 + 0.5 - 0.5) * 0.8;
    float threshold = 0.55;

    if (estimatedCloud < threshold - 0.1)
        return 0.0;


    // 軽量化：LODモードならここで終了
    // 影の計算や、遠くの雲の計算ではここを通る
    if (lodMode)
    {
        // 詳細ノイズがない分、少し密度が薄くなるのを補正するために係数を調整してもよいが、
        // 影用ならそのままでも自然に見えることが多い
        float finalDensityCheap = smoothstep(threshold, threshold + 0.3, estimatedCloud);
        return saturate(finalDensityCheap * 0.5);
    }


    // Detailノイズ（高周波成分）のサンプリング
    // Baseノイズのエッジを削ってディテールを作る
    float detail1 = g_volumeTex.SampleLevel(g_sampler, noiseCoord * 2.5, 0).r;
    float detail2 = g_volumeTex.SampleLevel(g_sampler, noiseCoord * 5.5, 0).r;
    
    float combinedNoise = baseNoise * 0.5 + detail1 * 0.35 + detail2 * 0.15;

    // 密度の合成
    float baseCloud = heightSignal + (combinedNoise - 0.5) * 0.8;
    float finalDensity = smoothstep(threshold, threshold + 0.3, baseCloud);

    return saturate(finalDensity * 0.5);
}

// ワールド座標復元
float3 GetWorldPositionFromDepth(float2 uv, float depth)
{
    float4 clipPos = float4(uv.x * 2.0 - 1.0, -(uv.y * 2.0 - 1.0), depth, 1.0);
    float4 worldPos = mul(clipPos, InvViewProj);
    return worldPos.xyz / worldPos.w;
}


// 定数
static const int MARCH_STEPS = 24; // レイマーチングの分割数

// ライティング用定数

static const float3 SUN_DIR = normalize(float3(0.2, 0.9, -0.3));
static const float3 AMBIENT_COLOR_TOP = float3(0.05, 0.1, 0.25); // 上空の環境光
static const float3 AMBIENT_COLOR_BOTTOM = float3(0.01, 0.01, 0.03); // 地上の環境光

// 雲の粒子による光の散乱を近似する関数
float HG(float cosAngle, float g)
{
    float g2 = g * g;
    return (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5) / 4.0 * 3.1415;
}

// ブルーノイズに近い特性を持つ擬似乱数、ディザリングに使用。
float GetIGN(float2 pixelPos)
{
    float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
    return frac(magic.z * frac(dot(pixelPos, magic.xy)));
}

//処理負荷軽減の為の雲のエリアとレイの交差
bool IntersectCloudLayer(float3 start, float3 dir, out float tNear, out float tFar)
{
    if (abs(dir.y) < 0.0001)
        dir.y = 0.0001;
    float t1 = (CLOUD_MIN_Y - start.y) / dir.y;
    float t2 = (CLOUD_MAX_Y - start.y) / dir.y;
    tNear = min(t1, t2);
    tFar = max(t1, t2);
    if (tFar < 0.0)
        return false;
    tNear = max(0.1, tNear);
    return tNear < tFar;
}


// ==========================================
// Main シェーダー
// ==========================================
OIT_PS_OUTPUT main(PS_INPUT input)
{
    OIT_PS_OUTPUT output;
    
    // --- レイのセットアップ ---
    float3 farPos = GetWorldPositionFromDepth(input.UV, 1.0);
    float3 startPos = EyePos.xyz;
    float3 rayDir = normalize(farPos - startPos);
    
    // gBufferからワールド座標を取得して、壁までの距離を計算
    float3 gBufferPos = g_worldPosTexture.Sample(g_sampler, input.UV).rgb;
    float distToWall = 10000.0;
    if (dot(gBufferPos, gBufferPos) > 0.001)
    {
        distToWall = length(gBufferPos - startPos);
    }

    float tCloudNear, tCloudFar;
    // 雲のエリアとの交差判定
    if (!IntersectCloudLayer(startPos, rayDir, tCloudNear, tCloudFar))
    {
        output.Accumulation = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.Revealage = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return output;
    }

    // 壁と雲の手前でクリップ
    tCloudFar = min(tCloudFar, distToWall);
    
    float maxMarchDist = 1000.0;
    // レイマーチングの最大距離を制限
    if (tCloudFar > tCloudNear + maxMarchDist)
    {
        tCloudFar = tCloudNear + maxMarchDist;
    }
    
    if (tCloudNear >= tCloudFar)
    {
        output.Accumulation = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.Revealage = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return output;
    }
      

    // レイマーチング準備 ----------------
    float marchDistance = tCloudFar - tCloudNear;
    
    // 軽量化 ステップ少なく
    int steps = MARCH_STEPS;
    
    float stepSize = marchDistance / float(steps);
    // 開始位置をピクセルごとにずらしてバンディングを軽減
    float dither = GetIGN(input.Position.xy + float2(r_time * 5.0, 0));
    float currentDist = tCloudNear + stepSize * dither;
    float3 currentPos = startPos + rayDir * currentDist;

    float totalDensity = 0.0;
    float3 accumulatedLight = 0.0;

    // 光の散乱係数
    float cosAngle = dot(rayDir, SUN_DIR);
    float phaseVal = lerp(HG(cosAngle, 0.5), HG(cosAngle, -0.2), 0.5);

    // 軽量化: LOD距離の設定
    // カメラからこの距離以上離れていたら、詳細ノイズを読まない
    float lodDistanceThreshold = 1000.0;


    // --- メインループ ---
    for (int i = 0; i < steps; i++)
    {
        // 雲の密度が十分に高くなったり範囲外
        if (currentDist >= tCloudFar || totalDensity >= 0.9)
            break;
            

        // 軽量化: 距離LOD判定
        // 現在位置がカメラから遠い場合は軽量モード(true)で呼ぶ
        bool isFar = currentDist > lodDistanceThreshold;

        // 密度取得
        float density = GetCloudDensity(currentPos, isFar);
        
        if (density > 0.001)
        {
            
            float3 sunStep = SUN_DIR * 5.0;
            
            // 現在の密度を代用して簡易的に陰影をつける
            float sunDensity = density;
            
            //密度による光の減衰
            float lightTransmittance = exp(-sunDensity * 10.0);

            //
            float3 directLight = CloudColor * lightTransmittance * phaseVal;
            
            // 高さによる環境光の変化
            float heightRate = saturate((currentPos.y - CLOUD_MIN_Y) / (CLOUD_MAX_Y - CLOUD_MIN_Y));
            float3 ambientLight = lerp(AMBIENT_COLOR_BOTTOM, AMBIENT_COLOR_TOP, heightRate) * 0.6;

            float3 stepColor = (directLight + ambientLight) * density;
            
            // アルファの蓄積
            float densityContribution = density * stepSize * 0.15;
            densityContribution = min(densityContribution, 1.0);
            
            // 色の蓄積
            accumulatedLight += stepColor * (1.0 - totalDensity) * stepSize * 0.2;
            totalDensity += (1.0 - totalDensity) * densityContribution;
        }

        currentPos += rayDir * stepSize;
        currentDist += stepSize;
    }

    // --- 最終出力 ---
    
    // バンディング隠しのためにノイズを乗せる
    float3 finalColor = accumulatedLight;
    finalColor += (dither - 0.5) * 0.01;
    finalColor = max(0.0, finalColor);

    float finalAlpha = totalDensity;
    
    if (finalAlpha < 0.001)
    {
        output.Accumulation = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.Revealage = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return output;
    }
    
    float oitAlpha = saturate(finalAlpha * 0.3); //0.3で調整//OIT統合の関係で薄いほうが違和感が少なく綺麗に見える

    matrix ViewProj = mul(Projection, View);

    float3 cloudSortPos = startPos + rayDir * tCloudFar; // Near -> Far に変更
    
    float4 clipPos = mul(float4(cloudSortPos, 1.0), ViewProj);
    float linearDepth = abs(clipPos.w);

    // 距離0での問題を防ぐため、ごくわずかにオフセット
    float weight = CalculateWeight(max(0.1f, linearDepth), oitAlpha);

    // Accumulationバッファが壊れないよう上限を抑える
    weight = min(weight, 1e4);


    output.Accumulation = float4(finalColor * weight, oitAlpha * weight);
    output.Revealage = float4(oitAlpha, oitAlpha, oitAlpha, 1.0);


    return output;
}
