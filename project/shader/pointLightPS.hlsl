#include "common.hlsl"

// G-Buffer
Texture2D g_albedoTexture : register(t0);
Texture2D g_normalTexture : register(t1);
Texture2D g_worldPosTexture : register(t2);
SamplerState g_sampler : register(s0);

struct P_PS_IN
{
    float4 Position : SV_POSITION;
    
    // VSから受け取るライト情報
    nointerpolation float3 LightPos : TEXCOORD1;
    nointerpolation float LightRange : TEXCOORD2;
    nointerpolation float3 LightColor : TEXCOORD3;
};

float4 main(P_PS_IN input) : SV_Target
{
    // スクリーンUV計算
    float2 resolution = float2(1920.0, 1080.0); // デフォルト値
    if (r_screenWidth > 0)
        resolution = float2(r_screenWidth, r_screenHeight);

    float2 uv = input.Position.xy / resolution;

    // G-Buffer読み込み
    float3 worldPos = g_worldPosTexture.Sample(g_sampler, uv).rgb;
    float3 normal = g_normalTexture.Sample(g_sampler, uv).rgb;
    float4 albedo = g_albedoTexture.Sample(g_sampler, uv);

    // 空判定 (WorldPosが0付近なら処理しない)
    if (dot(worldPos, worldPos) < 0.0001)
        discard;

    // ライトベクトル計算
    float3 toLight = input.LightPos - worldPos;
    float distToLight = length(toLight);

    // 範囲外チェック
    if (distToLight > input.LightRange)
        discard;

    float3 lightDir = normalize(toLight);

    // 減衰
    float att = 1.0 - smoothstep(0.0, input.LightRange, distToLight);
    
    // 拡散反射 (Diffuse)
    float diffuse = saturate(dot(normal, lightDir));

    // 鏡面反射
    float3 viewDir = normalize(zone.CameraPosition.xyz - worldPos);
    float3 halfVector = normalize(lightDir + viewDir);
    float specular = pow(saturate(dot(normal, halfVector)), 30.0);

    // 基本ライティング結果
    // input.LightColor (色*強度) を使う
    float3 finalColor = (albedo.rgb * diffuse + specular) * input.LightColor * att;

    // フォグ計算
    // カメラからピクセルまでの距離
    float distToCamera = distance(worldPos, zone.CameraPosition.xyz);

    // メインシェーダーと同じフォグ設定
    float fogStart = 400.0f;
    float fogEnd = 600.0f;
    
    // フォグ係数 (0=なし, 1=完全フォグ)
    float fogFactor = saturate((distToCamera - fogStart) / (fogEnd - fogStart));

    // ポイントライトは加算合成だからフォグ色を足すのではなく光を弱める。
    finalColor *= (1.0 - fogFactor);


    return float4(finalColor, 1.0);
}
