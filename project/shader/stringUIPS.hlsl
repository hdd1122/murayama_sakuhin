#include "common.hlsl"
//UI用MSDF
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//------------------------------------------------
// MSDF helper
//------------------------------------------------
float median3(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

//出力は１つのみ
PS_OUTPUT main(in PS_IN In)
{
    PS_OUTPUT Out;
    
    // スタイル分岐用
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
    // 背景（UIなので基本は透明にする）
    // ==============================
    float3 bgRGB = float3(0, 0, 0);
    float bgAlpha = 0.0;

    // ==============================
    // 文字ベース色
    // ==============================
    float3 textRGB = Material.Diffuse.rgb; // UIは頂点カラーよりマテリアル指定が楽かも？
    float textAlpha = textMask * Material.Diffuse.a; // 全体の透明度も考慮

    // ==============================
    // スタイル分岐（座標計算を修正）
    // ==============================

    if (g_TextStyle > 3.0f) // Energy Flow
    {
        // WorldPos.x ではなく TexCoord.x を使用
        float flow = sin(In.TexCoord.x * 10.0 + r_time * 5.0) * 0.5 + 0.5;
        
        float3 flowColor = float3(0.2, 0.8, 1.0);
        textRGB = lerp(textRGB, flowColor, flow * 0.6);
        textAlpha *= lerp(0.8, 1.0, flow);
    }
    else if (g_TextStyle > 2.0f) // Digital Noise
    {
        // WorldPos ではなく TexCoord を使用
        float noise = frac(sin(dot(In.TexCoord.xy * 20.0, float2(12.9898, 78.233))) * 43758.5453);
        
        // 時間更新
        noise = step(0.5 + sin(r_time * 4.0) * 0.1, noise);
        
        textRGB *= lerp(0.6, 1.2, noise);
        textAlpha *= noise;
    }
    else if (g_TextStyle > 1.0f) // Holo
    {
        // Scanline (Y軸)
        float scan = sin(In.TexCoord.y * 20.0 - r_time * 3.0) * 0.5 + 0.5;
        
        // Flicker
        float flicker = frac(sin(dot(In.TexCoord.xy, float2(12.9898, 78.233))) * 43758.5453);
        
        float holo = step(0.3, flicker) * scan; // step閾値調整
        
        // ホログラムは少し半透明にする
        textRGB *= holo;
        textAlpha *= 0.8 * scan; // スキャンラインに合わせて消える
    }

    // ==============================
    // 合成出力
    // ==============================
    // UIはアルファブレンド済みの色を出力することが多いですが、
    // ここでは単純に色とアルファを返します
    
    Out.SceneColor = float4(textRGB, textAlpha);

    // ==============================
    // Bloom
    // ==============================
    if (textMask > 0.01 && textAlpha > 0.01)
    {
        // ブルーム強度を調整したい場合はここで乗算
        Out.BloomContrib = bloomColor.Color * textAlpha;
    }
    else
    {
        Out.BloomContrib = float4(0, 0, 0, 0);
    }

    return Out;
}
