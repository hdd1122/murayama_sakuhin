#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//------------------------------------------------
// MSDF helper関数
//------------------------------------------------
float median3(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}


PS_OUTPUT main(in PS_IN In)
{
    PS_OUTPUT Out;  
    // 多分アンビエント使わないからaを使う
    float g_TextStyle = Material.Ambient.a;
    // ==============================
    // MSDF サンプル
    // ==============================
    float3 msdf = g_Texture.Sample(g_SamplerState, In.TexCoord).rgb;
    float sd = median3(msdf.r, msdf.g, msdf.b);
    float dist = sd - 0.5;

    // ==============================
    // 文字マスク（AA込み）
    // ==============================
    float w = fwidth(dist);
    float textMask = saturate(dist / w + 0.5);

    // ==============================
    // 背景（常に描画）
    // ==============================
    float3 bgRGB = Material.Ambient.rgb;
    float bgAlpha = 0.35;

    // ==============================
    // 文字ベース色
    // ==============================
    float3 textRGB = In.Diffuse.rgb;
    float textAlpha = textMask;

    // ==============================
    // スタイル分岐
    // ==============================
    if (g_TextStyle > 3.0f) // Energy Flow
    {
        float flow =
        sin(In.WorldPos.x * 10.0 + r_time * 5.0) * 0.5 + 0.5;

        float3 flowColor = float3(0.2, 0.8, 1.0);

        textRGB = lerp(textRGB, flowColor, flow * 0.6);
        textAlpha *= lerp(0.8, 1.0, flow);
    }
    else if (g_TextStyle > 2.0f) // Digital Noise
    {
        float noise =
        frac(sin(dot(In.WorldPos.xy * 20.0,
            float2(12.9898, 78.233))) * 43758.5453);

    // 時間でノイズを更新
        noise = step(0.5 + sin(r_time * 4.0) * 0.1, noise);

    // 文字内部だけ乱す
        textRGB *= lerp(0.6, 1.2, noise);
        textAlpha *= noise;
    }
    else if (g_TextStyle > 1.0f) // Holo
    {
        float scan =
            sin(In.WorldPos.y * 80.0 + r_time * 6.0) * 0.5 + 0.5;

        float flicker =
            frac(sin(dot(In.WorldPos.xy, float2(12.9898, 78.233)))
            * 43758.5453);

        float holo = step(0.3, flicker) * scan;

        textRGB *= holo;
        textAlpha *= holo;
    }
 
    // g_TextStyle == 0 は何もしない

    // ==============================
    // 背景 + 文字 合成
    // ==============================
    float3 finalRGB = lerp(bgRGB, textRGB, textMask);
    float finalAlpha = max(bgAlpha, textAlpha);

    Out.SceneColor = float4(finalRGB, finalAlpha);

    // ==============================
    // Bloom：文字だけ
    // ==============================
    if (textMask > 0.01)
    {
    Out.BloomContrib = bloomColor.Color;
    Out.BloomContrib.a *= 0.7f;
    }
    else
        Out.BloomContrib = float4(0, 0, 0, 0);

    return Out;
}
