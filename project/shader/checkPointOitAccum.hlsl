#include "common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// ブルーム設定 (b8)
cbuffer BloomColor : register(b8)
{
    // float4 Color; の形を想定 (wが強度)
    float4 g_BloomColor;
}

struct OIT_PS_OUTPUT
{
    float4 Accumulation : SV_Target0;
    float4 Revealage : SV_Target1;
};

OIT_PS_OUTPUT main(PS_IN In)
{
    OIT_PS_OUTPUT output;

    // -----------------------------------------------------------
    // 幾何学的なエフェクト
    // -----------------------------------------------------------
    // フレネル反射
    // 視線ベクトル
    float3 viewDir = normalize(zone.CameraPosition.xyz - In.WorldPos);
    // 法線
    float3 N = normalize(In.Normal);
    
    // 視線と法線の内積
    float NdotV = dot(N, viewDir);
    float fresnel = pow(1.0 - saturate(abs(NdotV)), 3.0); // 3乗して縁を鋭く

    // ワールドY座標と時間を使って、上へ登っていく縞模様を作る
    float scrollSpeed = 5.0; // 上昇スピード
    float stripeFreq = 2.0; // 縞の細かさ
    
    // sin波 (-1.0～1.0)を作る
    float scanline = sin(In.WorldPos.y * stripeFreq - zone.TotalTime * scrollSpeed);
    // 0.0 ~ 1.0 に変換し、少し細くする
    scanline = (scanline + 1.0) * 0.5;
    scanline = pow(scanline, 4.0); // コントラストを上げてビームっぽく


    // -----------------------------------------------------------
    // 色とアルファの決定
    // -----------------------------------------------------------

    float4 baseColor = Material.Diffuse;
    if (Material.TextureEnable)
    {
        baseColor.rgb = g_texture.Sample(g_sampler, In.TexCoord).rgb;
    }

    // アルファの加工:
    // 縁と走査線の部分を濃くし、
    // 中央の部分は少し透けさせることで、エネルギー体っぽく見せる
    float finalAlpha = baseColor.a;
    
    // フレネル合成: 縁ほど不透明に
    finalAlpha *= (0.3 + 0.7 * fresnel);
    
    // 走査線合成: 光の帯がある場所は不透明に
    finalAlpha = saturate(finalAlpha + scanline * 0.5);


    // -----------------------------------------------------------
    // ブルームの計算
    // -----------------------------------------------------------
    
    float intensity = g_BloomColor.w;
    float3 matEmission = g_BloomColor.rgb;
    float3 texEmission = float3(1.0, 1.0, 1.0);

    // エミッション計算
    float3 emissionColor = (texEmission * matEmission) * intensity;

    // 発光にもフレネルと走査線を乗算する
    float3 effectGlow = (fresnel * 2.0) + (scanline * 5.0); // 走査線は特に強く光らせる
    emissionColor *= effectGlow;


    // クランプ処理
    if (intensity <= 0.001)
    {
        baseColor.rgb = min(baseColor.rgb, float3(1.0, 1.0, 1.0));
        emissionColor = float3(0, 0, 0);
    }

    // 距離減衰
    float dist = distance(zone.CameraPosition.xyz, In.WorldPos);
    float distanceWeight = lerp(0.5, 1.0, 1.0 - smoothstep(50.0, 500.0, dist));
    emissionColor *= distanceWeight;


    // -----------------------------------------------------------
    // 出力
    // -----------------------------------------------------------

    float3 finalColor = baseColor.rgb + emissionColor;

    // WBOIT出力
    float weight = CalculateWeight(In.Position.z, finalAlpha);
    weight = min(weight, 1e4);

    output.Accumulation = float4(finalColor.rgb * finalAlpha, finalAlpha) * weight;
    output.Revealage = float4(finalAlpha, finalAlpha, finalAlpha, 1.0f);

    return output;
}
