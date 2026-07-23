// oitAccum_ps.hlsl

//現在ゾーンの描画特化になっている
#include "common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// 2つのレンダーターゲットに出力するための構造体
struct OIT_PS_OUTPUT
{
    float4 Accumulation : SV_Target0; // 色を蓄積するターゲット
    float4 Revealage : SV_Target1; // 透明度を蓄積するターゲット
};

//関数群
// (From webgl-noise by Stefan Gustavson)
float4 permute(float4 x)
{
    return fmod(((x * 34.0) + 1.0) * x, 289.0);
}
float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
//	Simplex 3D Noise 
//	by Ian McEwan, Stefan Gustavson
float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4 D = float4(0.0, 0.5, 1.0, 2.0);

// First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

// Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

    float3 x1 = x0 - i1 + 1.0 * C.xxx;
    float3 x2 = x0 - i2 + 2.0 * C.xxx;
    float3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
    i = fmod(i, 289.0);
    float4 p = permute(permute(permute(
             i.z + float4(0.0, i1.z, i2.z, 1.0))
           + i.y + float4(0.0, i1.y, i2.y, 1.0))
           + i.x + float4(0.0, i1.x, i2.x, 1.0));

// Gradients
    float n_ = 1.0 / 7.0;
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z);

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_);

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0.0, 0.0, 0.0, 0.0));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

//Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

// Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1),
                                dot(p2, x2), dot(p3, x3)));
}


OIT_PS_OUTPUT main(PS_IN In)
{
    OIT_PS_OUTPUT output;

    // --- ノイズ計算 ---
    float3 noiseCoord = In.WorldPos * 0.5f + float3(0.0, zone.TotalTime * 0.5f, 0.0);
    float noise = snoise(noiseCoord);
    noise = (noise + 1.0) * 0.5;

    // --- ベースカラー取得 ---
    float4 baseColor = Material.Diffuse;
    if (Material.TextureEnable)
    {
        baseColor.rgb = g_texture.Sample(g_sampler, In.TexCoord).rgb;
    }
    float finalAlpha = baseColor.a;


    // -----------------------------------------------------------
    // ブルーム（エミッション）設定の取得
    // -----------------------------------------------------------
    
    // 発光強度
    float intensity = bloomColor.Color.w;
    
    // 発光色
    float3 matEmission = bloomColor.Color.rgb;
    float3 texEmission = float3(1.0, 1.0, 1.0); // テクスチャは多分読まない

    // エミッション計算
    float3 emissionColor = (texEmission * matEmission) * intensity;

    // 0付近は光らせない
    if (intensity <= 0.001)
    {
        // 強制的にRGBを1.0以下に抑え込む
        baseColor.rgb = min(baseColor.rgb, float3(1.0, 1.0, 1.0));
    }


    // --- 距離減衰 ---
    float dist = distance(zone.CameraPosition.xyz, In.WorldPos);
    float BloomFadeStart = 50.0;
    float BloomFadeEnd = 200.0;
    float MinBloomIntensity = 0.4;

    float baseWeight = 1.0 - smoothstep(BloomFadeStart, BloomFadeEnd, dist);
    float distanceWeight = lerp(MinBloomIntensity, 1.0, baseWeight);
    
    emissionColor *= distanceWeight;


    // --- 最終カラー決定 ---
    float3 finalColor = baseColor.rgb + emissionColor;

    // ノイズ加算
    finalColor += (noise * 0.2f);


    //フォグ処理
    // カメラとピクセル（ワールド座標）の距離を計算
    float fogdist = distance(In.WorldPos, zone.CameraPosition.xyz);

    // フォグ係数を計算 (0.0 = フォグなし, 1.0 = 完全フォグ)
    // saturateで 0～1 にクランプするのが重要
    float fogFactor = saturate((fogdist - 400) / (600 - 400));

    // 元の色とフォグ色を線形補間 (Lerp)
    float3 fogColor = float3(0.7f, 0.8f, 0.9f);
    finalColor = lerp(finalColor, fogColor, fogFactor);
    
    // --- WBOIT出力 ---
    float weight = CalculateWeight(In.Position.z, finalAlpha);
    weight = min(weight, 1e4);

    output.Accumulation = float4(finalColor.rgb * finalAlpha, finalAlpha) * weight;
    output.Revealage = float4(finalAlpha, finalAlpha, finalAlpha, 1.0f);

    return output;
}

